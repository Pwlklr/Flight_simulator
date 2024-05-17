#pragma once

#include <fstream>
#include "RigidBody.h"
#include "ModelLoader.h"

bool logFlight = true;


namespace Atmosphere { // based on International Standard Atmosphere (ISA)

// get temperture in kelvin
float get_air_temperature(float altitude) {
    assert(0.0f <= altitude && altitude <= 11000.0f);
    return 288.15f - 0.0065f * altitude;
}

// only accurate for altitudes < 11km
float get_air_density(float altitude) {
    assert(0.0f <= altitude && altitude <= 11000.0f);
    float temperature = get_air_temperature(altitude);
    float pressure = 101325.0f * std::pow(1 - 0.0065f * (altitude / 288.15f), 5.25f);
    return 0.00348f * (pressure / temperature);
}
const float sea_level_air_density = get_air_density(0.0f);

}; // namespace Atmosphere

struct Airfoil {
    float min_alpha, max_alpha, lift_max;
    std::vector<glm::vec3> data;

    Airfoil(const std::vector<glm::vec3> &curve) :
            data(curve) {
        min_alpha = curve[0].x;
        max_alpha = curve[curve.size() - 1].x;
        lift_max = 0;
        for (const auto &point : curve) {
            lift_max = std::max(lift_max, point.y);
        }
    }

    std::pair<float, float> getAirfoilDataInterpolated(float alpha) const {
        // Binary search to find the closest data points
        auto search = std::lower_bound(data.begin(), data.end(), glm::vec3(alpha, 0.0f, 0.0f),
                [](const glm::vec3 &lhs, const glm::vec3 &rhs) {
                    return lhs.x < rhs.x;
                });

        // If the angle is not in data vector
        if (search == data.end()) {
            return { 0.0f, 0.5f }; // Example values, require further tests
        }

        // If the angle is exactly in data vector
        if (search->x == alpha) {
            return { search->y, search->z };
        }

        // If the angle is between two data points, perform linear interpolation
        if (search != data.begin()) {
            auto prev = search - 1;

            // Perform linear interpolation
            float alpha_interpolated = (alpha - prev->x) / (search->x - prev->x);
            float lift_interpolated = glm::mix(prev->y, search->y, alpha_interpolated);
            float drag_interpolated = glm::mix(prev->z, search->z, alpha_interpolated);

            return { lift_interpolated, drag_interpolated };
        }

        return { 0.5f, 0.5f }; // Example values, require further tests
    }

    std::pair<float, float> getAirfoilData(float alpha) const {
        float minDiff = std::numeric_limits<float>::max();

        glm::vec3 closestPoint;
        for (const auto &point : data) {
            float diff = std::abs(point.x - alpha);
            if (diff < minDiff) {
                minDiff = diff;
                closestPoint = point;
            }
        }
        return { closestPoint.y, closestPoint.z };
    }

};

class Wing {
private:
    const Airfoil *airfoil; // pointer to aerodynamic profile of a wing: http://airfoiltools.com/
    glm::vec3 center_of_pressure;
    float area; // wing's surface area
    float chord; // length of the chord
    float wingspan;
    glm::vec3 normal; // normal vector to the wing
    float wing_aspect_ratio;
    float flap_ratio; // [0,1]

public:
    Wing(const glm::vec3 &position, float span, float chord, const Airfoil *airfoil, const glm::vec3 &normal, float flap_ratio) :
            airfoil(airfoil),
            center_of_pressure(position),
            area(span * chord),
            chord(chord),
            wingspan(span),
            normal(normal),
            wing_aspect_ratio(span * span / area),
            flap_ratio(flap_ratio) {}

    void calculate_forces(RigidBody *rigid_body, float delta_time, float control_input) {
        std::ofstream wingDebug("wingDebug.txt", std::ios::app);

        glm::vec3 wing_velocity = rigid_body->get_point_velocity(center_of_pressure);

        float speed = glm::length(wing_velocity); // gets the velocity of the center of pressure for the wing

        glm::vec3 drag_direction = glm::normalize(-wing_velocity); // direction of drag is perpendicular to velocity

        wingDebug << " wing_velocity " << speed << " drag_direction " << drag_direction.x << " drag_direction " << drag_direction.y << " drag_direction " << drag_direction.z << "\n";

        glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction)); // glm::vec3 lift_direction = glm::normalize(glm::cross(drag_direction, normal)); <- requires further testing

        wingDebug << " wing_velocity " << speed << " drag_direction " << drag_direction.x << " drag_direction " << drag_direction.y << " drag_direction " << drag_direction.z << 
        " lift_direction " << lift_direction.x << " lift_direction " << lift_direction.y << " lift_direction " << lift_direction.z << "\n";


        float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, normal))); // angle between chord line and air flow

        std::pair<float, float> airfoilData = airfoil->getAirfoilDataInterpolated(angle_of_attack); // .first: lift_coeff, .second drag_coeff

        wingDebug << "angleofattack " << angle_of_attack << " lc " << airfoilData.first << " dc " << airfoilData.second << "\n";

        if (flap_ratio) {
            float delta_lift_coeff = sqrt(flap_ratio) * airfoil->lift_max * control_input; // lift coefficient changes based on flap deflection
            airfoilData.first += delta_lift_coeff;
        }

        // induced drag, increases with lift
        float induced_drag_coeff = (std::pow(airfoilData.first, 2)) / (PI * wing_aspect_ratio); // (PI * wing_aspect_ratio * efficiency_ratio) <- test?
        airfoilData.second += induced_drag_coeff;

        float air_density = Atmosphere::get_air_density(rigid_body->position.y);

        float dynamic_pressure = 0.5f * std::pow(speed, 2) * air_density * area;

        glm::vec3 lift = lift_direction * airfoilData.first * dynamic_pressure;
        glm::vec3 drag = drag_direction * airfoilData.second * dynamic_pressure;

        rigid_body->add_force_at_point(lift + drag, center_of_pressure);
    }
};

struct Engine {
    float throttle = 1.0f, thrust; // thrust is constant, throttle is in the range of [0, 1]

    Engine(float thrust) :
            thrust(thrust) {}

    void apply_force(RigidBody *rigid_body, float control_throttle) {
        rigid_body->add_relative_force(FORWARD * (control_throttle * thrust)); // thrust is applied to the center of gravity and does not produce torque
    }
};

class Airplane : public RigidBody {
public:
    Engine engine;
    std::vector<Wing> wing_elements;
    float input, control_throttle, flight_time;

    Airplane(float mass, const Engine &engine, const glm::mat3 &inertia, const std::vector<Wing> &wings) :
            RigidBody(mass, inertia),
            engine(engine),
            wing_elements(wings),
            input(0.0f),
            control_throttle(1.0f),
            flight_time(0.0f)
        {
            // this->mass = mass;
            // this->inertia = inertia;
        }

    void update(float delta_time) {

        if (logFlight) { // made for debugging this shit

            std::ofstream log_file("log_file.txt", std::ios::app);
            flight_time += delta_time;

            log_file << flight_time << " pos " << position.y << " speed " << glm::length(velocity) << " force " << getForce().x << " force " << getForce().y << " force " << getForce().z << "\n";

            for (Wing &wing : wing_elements) {
                wing.calculate_forces(this, delta_time, input);
            }
            engine.apply_force(this, control_throttle);

            log_file << flight_time << " pos " << position.y << " speed " << glm::length(velocity) << " force " << getForce().x << " force " << getForce().y << " force " << getForce().z << " torque " << getTorque().x << " torque " << getTorque().y << " torque " << getTorque().z << "\n";

            RigidBody::UpdateBody(delta_time);


        } else {

            for (Wing &wing : wing_elements) {
                wing.calculate_forces(this, delta_time, input);
            }

            engine.apply_force(this, control_throttle);
            RigidBody::UpdateBody(delta_time);
        }
    }

    void drawAirplane(float delta_time, ShaderProgram *sp, Mesh *airplaneMesh, GLuint tex0, GLuint tex1) {

        // update(delta_time);

        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M, position);
        M = M * glm::mat4_cast(orientation); // changing the quaternion orientation into mat4

        glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

        glEnableVertexAttribArray(sp->a("vertex"));
        glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, airplaneMesh->vertices.data());

        glEnableVertexAttribArray(sp->a("normal"));
        glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, airplaneMesh->normals.data());

        glEnableVertexAttribArray(sp->a("texCoord0"));
        glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, airplaneMesh->texCoords.data());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1); // mo¿na rozwin¹æ potem o inne mapowania

        glDrawArrays(GL_TRIANGLES, 0, airplaneMesh->vertices.size());

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));
    }
};

