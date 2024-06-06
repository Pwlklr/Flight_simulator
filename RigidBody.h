#pragma once
#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <numeric>
#include <type_traits>
#include <variant>
#include <vector>
#include <math.h>

#include "constants.h"

class RigidBody {
private:
    glm::vec3 mw_force{}; // model's force in world space
    glm::vec3 mb_torque{}; // model's torque in bodu space

public:
    float mass;                                 // mass of the body in kg
    glm::vec3 position{};                       // world space
    glm::quat orientation{};                    // world space, using the quaternion representation
    glm::vec3 velocity{};                       // world space, m/s
    glm::vec3 angular_velocity{};               // body space, radians/second
    glm::mat3 inertia{}, body_space_inertia{};     // inertia tensor, body space

    RigidBody(float _mass, const glm::mat3 &_inertia, const glm::vec3 &_position) :
            mass(_mass),
            position(_position),
            orientation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
            velocity(glm::vec3(600.0f / 3.6f, 0.0f, 0.0f)),
            angular_velocity(glm::vec3(0.0f)),
            inertia(_inertia),
            body_space_inertia(glm::inverse(_inertia)) { }

    RigidBody(float mass, const glm::mat3 &inertia, glm::vec3 initial_position, glm::quat initial_orientation, glm::vec3 initial_velocity) : // this constructor will be used for the Missle class
            mass(mass),
            position(initial_position),
            orientation(initial_orientation),
            velocity(initial_velocity),
            angular_velocity(glm::vec3(0.0f)),
            inertia(inertia),
            body_space_inertia(glm::inverse(inertia)) {}

    glm::vec3 vector_to_world(const glm::vec3 &direction); // transforms the direction vector from body space to world space
    glm::vec3 vector_to_body(const glm::vec3 &direction); // transforms the direction vector from world space to body space
    glm::vec3 get_point_velocity(const glm::vec3 &point);       // calculates the sum of angular velocity and the velocity of a point in body space
    void add_relative_force(const glm::vec3 &force);            // used to simulate forces that are not applied at a specific point (mostly used for calculating the thrust vector effect on the whole body). Used for translation, doesnt affect the body's rotation
    void add_force_at_point(const glm::vec3 &force, const glm::vec3 &point); // adds force and torque to the accumulators m_force and torque used to calculate the resulant force on the entire body. Mostly for rotation
    void add_torque(const glm::vec3 &torque);
    glm::vec3 getForce() { return mw_force; }
    glm::vec3 getTorque() { return mb_torque; }


    virtual void UpdateBody(float deltaTime); // updates the position and orientation of the whole body
    virtual void update(float deltaTime); // updates the position and orientation of the whole body
};

#endif