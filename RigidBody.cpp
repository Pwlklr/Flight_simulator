#include "RigidBody.h"

// Constants for maximum speeds
const float MAX_FORWARD_SPEED = 300.0f; // max forward speed in m/s
const float MAX_ANGULAR_SPEED = 1.0f; // max angular speed in radians/s

glm::vec3 RigidBody::vector_to_world(const glm::vec3 &direction) {
    return orientation * direction;
}
glm::vec3 RigidBody::vector_to_body(const glm::vec3 &direction) {
    return glm::inverse(orientation) * direction;
}

glm::vec3 RigidBody::get_point_velocity(const glm::vec3& point) {
    return vector_to_body(velocity) + glm::cross(angular_velocity, point);
}

void RigidBody::add_relative_force(const glm::vec3 &force) {
    mw_force += vector_to_world(force);
}

void RigidBody::add_force_at_point(const glm::vec3 &force, const glm::vec3 &point) {
    mw_force += vector_to_world(force);
    mb_torque += glm::cross(point, force);
}
void RigidBody::add_torque(const glm::vec3& torque) {
    mb_torque += torque;
}

void RigidBody::UpdateBody(float deltaTime, float pitch, float roll, float yaw) {

    // calculating the position of the model using, a = F / M; V = a / deltaT; S = v / T; 
    glm::vec3 acceleration = mw_force / mass;
    acceleration.y -= G;
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

    mb_torque += glm::vec3(roll, yaw, roll);

    // calculating the orientation using the dynamics of a rigid body
    angular_velocity += body_space_inertia * (mb_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * deltaTime; // 2nd Newton's principle of rotational dynamics
    orientation += (orientation * glm::quat(0.0f, angular_velocity)) * (deltaTime / 2);
    orientation = glm::normalize(orientation);

    // Clamp the forward speed and angular velocity
    if (glm::length(velocity) > MAX_FORWARD_SPEED) {
        velocity = glm::normalize(velocity) * MAX_FORWARD_SPEED;
    }
    if (glm::length(angular_velocity) > MAX_ANGULAR_SPEED) {
        angular_velocity = glm::normalize(angular_velocity) * MAX_ANGULAR_SPEED;
    }

    mw_force = glm::vec3(0.0f);
    mb_torque = glm::vec3(0.0f);
}

void RigidBody::update(float deltaTime) {
    glm::vec3 acceleration = mw_force / mass;
    acceleration.y -= G;
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;
    angular_velocity += body_space_inertia * (mb_torque - glm::cross(angular_velocity, inertia * angular_velocity)) * deltaTime;
    orientation += (orientation * glm::quat(0.0f, angular_velocity)) * (0.5f * deltaTime);
    orientation = glm::normalize(orientation);

    // Clamp the forward speed and angular velocity
    if (glm::length(velocity) > MAX_FORWARD_SPEED) {
        velocity = glm::normalize(velocity) * MAX_FORWARD_SPEED;
    }
    if (glm::length(angular_velocity) > MAX_ANGULAR_SPEED) {
        angular_velocity = glm::normalize(angular_velocity) * MAX_ANGULAR_SPEED;
    }

    mw_force = glm::vec3(0.0f);
    mb_torque = glm::vec3(0.0f);
}