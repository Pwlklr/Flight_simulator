#pragma once

#include "RigidBody.h"

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

    std::pair<float, float> getAirfoilData(float alpha) const {
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

        return { 0.0f, 0.5f }; // Example values, require further tests
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
    float flap_ratio; // [-1,1] how does a certain control surface behave

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
        glm::vec3 wing_velocity = rigid_body->get_point_velocity(center_of_pressure);
        float speed = glm::length(wing_velocity); // gets the velocity of the center of pressure for the wing

        glm::vec3 drag_direction = glm::normalize(-wing_velocity); // direction of drag is perpendicular to velocity

        glm::vec3 lift_direction = glm::normalize(glm::cross(drag_direction, normal)); //  glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction)); <- requires ffurther testing

        float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, normal))); // angle between chord line and air flow

        std::pair<float, float> airfoilData = airfoil->getAirfoilData(angle_of_attack); // .first: lift_coeff, .second drag_coeff

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

    void apply_force(RigidBody *rigid_body) {
        rigid_body->add_relative_force(FORWARD * (throttle * thrust)); // thrust is applied to the center of gravity and does not produce torque
    }
};

class Airplane : public RigidBody {
public:
    Engine engine;
    std::vector<Wing> wing_elements;
    float input;

    Airplane(float mass, const Engine &engine, const glm::mat3 &inertia, const std::vector<Wing> &wings) :
            RigidBody(),
            engine(engine),
            wing_elements(wings),
            input(0.0f)
        {
            this->mass = mass;
            this->inertia = inertia;
        }

    void update(float delta_time) {
        engine.apply_force(this);
        for (Wing& wing : wing_elements) {
            wing.calculate_forces(this, delta_time, input);
        }
        RigidBody::UpdateBody(delta_time);
    }
};

