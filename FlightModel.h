#pragma once

#include <fstream>
#include "RigidBody.h"
#include "ModelLoader.h"

bool logFlight = false;


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
    glm::vec3 initial_normal; // normal vector to the wing
    float wing_aspect_ratio;
    float flap_ratio; // [0,1]
    float control_input = 0.0f; // [-1, 1]


public:
    glm::vec3 normal; // normal vector to the wing

    Wing(const glm::vec3 &_position, float span, float chord, const Airfoil *airfoil, const glm::vec3 &normal, float flap_ratio) :
            airfoil(airfoil),
            center_of_pressure(_position),
            area(span * chord),
            chord(chord),
            wingspan(span),
            initial_normal(normal),
            normal(normal),
            wing_aspect_ratio(span * span / area),
            flap_ratio(flap_ratio),
            control_input(0.0f)
            {}

    void setControlInput(float _control_input) {
        this->control_input = _control_input;
    }

    void calculate_forces(RigidBody *rigid_body, float delta_time) {
        // std::ofstream wingDebug("wingDebug.txt", std::ios::app);

        glm::vec3 wing_velocity = rigid_body->get_point_velocity(center_of_pressure);

        float speed = glm::length(wing_velocity); // gets the velocity of the center of pressure for the wing

        glm::vec3 drag_direction = glm::normalize(-wing_velocity); // direction of drag is perpendicular to velocity

        glm::vec3 lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction)); // glm::vec3 lift_direction = glm::normalize(glm::cross(drag_direction, normal)); <- requires further testing

        //  << " wing_velocity " << speed << " drag_direction " << drag_direction.x << " drag_direction " << drag_direction.y << " drag_direction " << drag_direction.z << 
        // " lift_direction " << lift_direction.x << " lift_direction " << lift_direction.y << " lift_direction " << lift_direction.z << "\n";


        float angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, normal))); // angle between chord line and air flow

        std::pair<float, float> airfoilData = airfoil->getAirfoilDataInterpolated(angle_of_attack); // .first: lift_coeff, .second drag_coeff

        // wingDebug << "normal" << normal.x << " " << normal.y << " " << normal.z << " initial " << initial_normal.x << " " << initial_normal.y << " " << initial_normal.z << "\n";

        if (flap_ratio) {
            float delta_lift_coeff = sqrt(flap_ratio) * airfoil->lift_max * control_input; // lift coefficient changes based on flap deflection
            airfoilData.first += delta_lift_coeff;
        }

        // induced drag, increases with lift
        float induced_drag_coeff = (std::pow(airfoilData.first, 2)) / (PI * wing_aspect_ratio); // (PI * wing_aspect_ratio * efficiency_ratio) <- test?
        airfoilData.second += induced_drag_coeff * 100;

        float air_density = Atmosphere::get_air_density(rigid_body->position.y);

        float dynamic_pressure = 0.5f * std::pow(speed, 2) * air_density * area;

        glm::vec3 lift = lift_direction * airfoilData.first * dynamic_pressure;
        glm::vec3 drag = drag_direction * airfoilData.second * dynamic_pressure;

        rigid_body->add_force_at_point(lift + drag, center_of_pressure);
        // normal = initial_normal;
    }
};

struct Engine {
    float thrust; // thrust is constant, throttle is in the range of [0, 1]

    Engine(float thrust) :
            thrust(thrust)
    {}

    void apply_force(RigidBody *rigid_body, float control_throttle) {
        rigid_body->add_relative_force(FORWARD * (control_throttle * thrust)); // thrust is applied to the center of gravity and does not produce torque
    }
    void apply_force(RigidBody *rigid_body) { // used for missle simulation, no throttle control
        rigid_body->add_relative_force(FORWARD * thrust); // thrust is applied to the center of gravity and does not produce torque
    }
};

class Missile : public RigidBody {
public:
    Engine engine;
    float flight_time, max_flight_time, engine_time;

    Missile(float mass, const Engine &engine, const glm::mat3 &inertia, glm::vec3 initial_position, glm::quat initial_orientation, glm::vec3 initial_velocity, float max_flight_time = 5.0f) :
            RigidBody(mass, inertia, initial_position, initial_orientation, initial_velocity),
            engine(engine),
            flight_time(0.0f),
            max_flight_time(max_flight_time) {}

    void update(float delta_time) {
        engine.apply_force(this);
        RigidBody::UpdateBody(delta_time);
        flight_time += delta_time;
    }

    void drawMissile(float delta_time, ShaderProgram *sp, Mesh *airplaneMesh, GLuint tex0, GLuint tex1) {

        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M, position);
        M = M * glm::mat4_cast(orientation);
        M = glm::rotate(M, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        M = glm::scale(M, glm::vec3(4.0f, 4.0f, 4.0f)); // changing the quaternion orientation into mat4

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
        glBindTexture(GL_TEXTURE_2D, tex1); // mozna rozwinac potem o inne mapowania

        glDrawArrays(GL_TRIANGLES, 0, airplaneMesh->vertices.size());

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));
    }
};

class Airplane : public RigidBody {
public:
    Engine engine;
    std::vector<Missile> missiles;
    std::vector<Wing> wing_elements;
    float input, control_throttle, flight_time, missile_offset = 20.0f;
    short aileron, rudder, elevator;
    Engine missileEngine;

    Airplane(float mass, const Engine &engine, const glm::mat3 &inertia, const std::vector<Wing> &wings) :
            RigidBody(mass, inertia),
            engine(engine),
            wing_elements(wings),
            input(1.0f),
            control_throttle(1.0f),
            flight_time(0.0f),
            aileron(0),
            rudder(0),
            elevator(0),
            missileEngine(10000.0f)
        {}

    void fireMissile() {
        glm::vec3 missile_position = this->position + (this->orientation * glm::vec3(1.0f, 1.0f, 1.0f)) + glm::vec3(0.0f, -2.5f, missile_offset); // Przyk�adowe miejsce startu rakiety
        glm::vec3 missile_velocity = this->velocity + (this->orientation * glm::vec3(1.0f, 1.0f, 1.0f)); // Przyk�adowa pr�dko�� pocz�tkowa rakiety
        Missile new_missile(100.0f, missileEngine, glm::mat3(1.0f), missile_position, this->orientation, missile_velocity);
        missiles.push_back(new_missile);
        missile_offset *= -1;
    }

    void update(float delta_time) {

        if (logFlight) { // made for debugging this shit

            std::ofstream log_file("log_file.txt", std::ios::app);
            flight_time += delta_time;

            log_file << flight_time << " pos " << position.y << " speed " << glm::length(velocity) << " force " << getForce().x << " force " << getForce().y << " force " << getForce().z << "\n";

            for (Wing &wing : wing_elements) {
                wing.calculate_forces(this, delta_time);
            }
            engine.apply_force(this, control_throttle);

            log_file << flight_time << " pos " << position.y << " speed " << glm::length(velocity) << " force " << getForce().x << " force " << getForce().y << " force " << getForce().z << " torque " << getTorque().x << " torque " << getTorque().y << " torque " << getTorque().z << "\n";

            RigidBody::UpdateBody(delta_time);


        } else {
            // std::ofstream log_file("log_file.txt", std::ios::app);
            // log_file << aileron << " " << elevator << " " << rudder << "\n";

           if (aileron == -1) {
               wing_elements[0].setControlInput(-1);
               wing_elements[1].setControlInput(1);
            } else if (aileron == 1) {
               wing_elements[0].setControlInput(1);
                wing_elements[1].setControlInput(-1);
            } else {
                wing_elements[0].setControlInput(0.0f);
                wing_elements[1].setControlInput(0.0f);
            }

            if (elevator == -1) {
                wing_elements[2].setControlInput(1);
            } else if (elevator == 1) {
                wing_elements[2].setControlInput(-1);
            } else {
                wing_elements[2].setControlInput(0.0f);
            }

            if (rudder == -1) {
                wing_elements[3].setControlInput(1);
            } else if (rudder == 1) {
                wing_elements[3].setControlInput(-1);
            } else {
                wing_elements[3].setControlInput(0.0f);
            }

            for (Wing &wing : wing_elements) {
                wing.calculate_forces(this, delta_time);
            }

            engine.apply_force(this, control_throttle);

            // Update rockets
            for (auto it = missiles.begin(); it != missiles.end();) {
                it->update(delta_time);
                if (it->flight_time > it->max_flight_time) {
                    it = missiles.erase(it);
                } else {
                    it++;
                }
            }

            RigidBody::UpdateBody(delta_time);
        }
    }

  void drawAirplane(float delta_time, ShaderProgram *sp, Mesh *airplaneMesh, Mesh *missileMesh, GLuint tex0, GLuint tex1) {

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
      glBindTexture(GL_TEXTURE_2D, tex1); // mozna rozwinac potem o inne mapowania

      glDrawArrays(GL_TRIANGLES, 0, airplaneMesh->vertices.size());

      // Drawing rockets
      for (Missile &missile : missiles) {
          missile.drawMissile(delta_time, sp, missileMesh, tex0, tex1);
      }

      glDisableVertexAttribArray(sp->a("vertex"));
      glDisableVertexAttribArray(sp->a("normal"));
      glDisableVertexAttribArray(sp->a("texCoord0"));
  }
};


class Helicopter{
public:
    float mainRotorSpeed;
    float tailRotorSpeed;
    float mass;
    Engine engine;
    Engine missileEngine;
    std::vector<Missile> missiles;
    glm::vec3 position{};
    glm::quat orientation{};
    glm::vec3 velocity{};
    glm::vec3 angular_velocity{};
    float maxTiltAngle = glm::radians(75.0f); // Maksymalny kąt wychylenia w radianach
    float stabilizationRate = 0.1f;
    float turn_rate = glm::radians(1000.0f);
    float throttle = 1.0f;
    float missile_offset = 40.0f;

    Helicopter(float mass, Engine &engine, const glm::mat3 &inertia, glm::vec3 initial_position) :
            mass(mass),
            mainRotorSpeed(0.0f),
            tailRotorSpeed(0.0f),
            engine(engine),
            position(initial_position),
            missileEngine(10000.0f)
        {}

     void fireMissile() {
        glm::vec3 missile_position = this->position + (this->orientation * glm::vec3(1.0f, 1.0f, 1.0f)) + glm::vec3(-5.0f, -20.5f, missile_offset); // Przykładowe miejsce startu rakiety
        glm::vec3 missile_velocity = this->velocity + (this->orientation * glm::vec3(1.0f, 1.0f, 1.0f)); // Przykładowa prędkość początkowa rakiety
        Missile new_missile(100.0f, missileEngine, glm::mat3(1.0f), missile_position, this->orientation, missile_velocity);
        missiles.push_back(new_missile);
        missile_offset *= -1;
    }

    void updateHeli(float deltaTime) {

        // Update orientation
        glm::quat deltaOrientation = glm::quat(1.0f, angular_velocity * deltaTime);
        orientation = glm::normalize(glm::cross(deltaOrientation, orientation));

        // Clamp orientation to max tilt angle
        // glm::vec3 euler = glm::eulerAngles(orientation);
        // euler.x = glm::clamp(euler.x, -maxTiltAngle, maxTiltAngle);
        // euler.z = glm::clamp(euler.z, -maxTiltAngle, maxTiltAngle);
        // orientation = glm::quat(euler);
        

        glm::vec3 thrust = engine.thrust * throttle * (orientation * UP);

        glm::vec3 acceleration = thrust / mass;
        acceleration.y -= G;


        // Aktualizacja prędkości i pozycji
        velocity += acceleration * deltaTime;
        position += velocity * deltaTime;

        // Update rockets
        for (auto it = missiles.begin(); it != missiles.end();) {
            it->update(deltaTime);
            if (it->flight_time > it->max_flight_time) {
                it = missiles.erase(it);
            } else {
                it++;
            }
        }

        angular_velocity = glm::vec3(0.0f);
    }

    void applyInput(char key, float deltaTime) {

        switch (key) {
        case 'w':
            angular_velocity -= glm::vec3(0.0f, 0.0f, turn_rate * deltaTime);
            break;
        case 's':
            angular_velocity += glm::vec3(0.0f, 0.0f, turn_rate * deltaTime);
            break;
        case 'a':
            angular_velocity -= glm::vec3(turn_rate * deltaTime * 1.2, 0.0f, 0.0f);
            break;
        case 'd':
            angular_velocity += glm::vec3(turn_rate * deltaTime * 1.2 , 0.0f, 0.0f);
            break;
        case 'q':
            angular_velocity += glm::vec3(0.0f, turn_rate * deltaTime * 1.5, 0.0f);
            break;
        case 'e':
            angular_velocity -= glm::vec3(0.0f, turn_rate * deltaTime * 1.5, 0.0f);
            break;
        }
    }

    void drawHelicopter(float delta_time, ShaderProgram *sp, Mesh *bodyMesh, Mesh *rotorMesh, Mesh *missileMesh, GLuint tex0, GLuint tex1) {
        // Rysowanie ciała helikoptera
        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M, position);
        M = M * glm::mat4_cast(orientation);
        M = glm::scale(M, glm::vec3(0.3f, 0.3f, 0.3f));

        glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

        glEnableVertexAttribArray(sp->a("vertex"));
        glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, bodyMesh->vertices.data());

        glEnableVertexAttribArray(sp->a("normal"));
        glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, bodyMesh->normals.data());

        glEnableVertexAttribArray(sp->a("texCoord0"));
        glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, bodyMesh->texCoords.data());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1);

        glDrawArrays(GL_TRIANGLES, 0, bodyMesh->vertices.size());

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));

        // Rysowanie głównego śmigła
        static float rotorAngle = 0.0f;
        rotorAngle += 10.0f * delta_time;
        if (rotorAngle > 2 * PI) rotorAngle -= 2 * PI;

        // Resetujemy macierz transformacji wirnika
        glm::mat4 rotorTransform = glm::mat4(1.0f);
        rotorTransform = glm::translate(rotorTransform, this->position);
        rotorTransform *= glm::mat4_cast(this->orientation);
        rotorTransform = glm::translate(rotorTransform, glm::vec3(-7.0f, -30.0f, 0.0f));
        rotorTransform = glm::rotate(rotorTransform, rotorAngle, glm::vec3(0.0f, 1.0f, 0.0f)); 
        rotorTransform = glm::scale(rotorTransform, glm::vec3(12.0f, 12.0f, 12.0f));

        glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(rotorTransform));

        glEnableVertexAttribArray(sp->a("vertex"));
        glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, rotorMesh->vertices.data());

        glEnableVertexAttribArray(sp->a("normal"));
        glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, rotorMesh->normals.data());

        glEnableVertexAttribArray(sp->a("texCoord0"));
        glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, rotorMesh->texCoords.data());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1);

        glDrawArrays(GL_TRIANGLES, 0, rotorMesh->vertices.size());

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));

        // Rysowanie tylnego śmigła
        glm::mat4 tailRotorTransform = glm::mat4(1.0f);
        tailRotorTransform = glm::translate(tailRotorTransform, this->position);
        tailRotorTransform *= glm::mat4_cast(this->orientation);
        tailRotorTransform = glm::translate(tailRotorTransform, glm::vec3(-125.0, 2.5f, 10.0f));
        tailRotorTransform = glm::rotate(tailRotorTransform, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        tailRotorTransform = glm::rotate(tailRotorTransform, rotorAngle * 1.7f, glm::vec3(0.0f, 1.0f, 0.0f));
        tailRotorTransform = glm::scale(tailRotorTransform, glm::vec3(3.5f, 3.5f, 3.5f));

        glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(tailRotorTransform));

        glEnableVertexAttribArray(sp->a("vertex"));
        glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, rotorMesh->vertices.data());

        glEnableVertexAttribArray(sp->a("normal"));
        glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, rotorMesh->normals.data());

        glEnableVertexAttribArray(sp->a("texCoord0"));
        glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, rotorMesh->texCoords.data());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1);

        glDrawArrays(GL_TRIANGLES, 0, rotorMesh->vertices.size());

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));

        // Drawing rockets
        for (Missile &missile : missiles) {
            missile.drawMissile(delta_time, sp, missileMesh, tex0, tex1);
        }
    }
};

