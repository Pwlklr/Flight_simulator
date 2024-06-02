#ifndef AIRPLANE_H
#define AIRPLANE_H


#include "constants.h"
#include "ModelLoader.h"
#include "shaderprogram.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Airplane {
private:
    glm::vec3 position;
    glm::quat orientation;
    float speed;
    float maxSpeed;
    float acceleration;
    float rollSpeed;
    float pitchSpeed;
    float yawSpeed;

public:
    Airplane() :
            position(0.0f, 0.0f, 0.0f),
            orientation(1.0f, 0.0f, 0.0f, 0.0f),
            speed(0.0f),
            maxSpeed(300.0f),
            acceleration(5.0f),
            rollSpeed(0.1f),
            pitchSpeed(0.01f),
            yawSpeed(0.01f) {}

    glm::vec3 getPosition() {
        return position;
    }

    glm::vec3 getForwardDirection() {
        return orientation * FORWARD;
    }

    glm::vec3 getUpDirection() {
        return orientation * UP;
    }

    glm::vec3 getRightDirection() {
        return orientation * RIGHT;
    }

    glm::mat4 getRotationMatrix() {
        return glm::toMat4(orientation);
    }

    void update(float deltaTime) {
        speed += acceleration * deltaTime;
        if (speed > maxSpeed) speed = maxSpeed;
        if (speed < 0.0f) speed = 0.0f;

        position += getForwardDirection() * speed * deltaTime;
    }

    void roll(float direction) {
        float angle = rollSpeed * direction;
        orientation = glm::rotate(orientation, angle, FORWARD);
    }

    void pitch(float direction) {
        float angle = pitchSpeed * direction;
        orientation = glm::rotate(orientation, angle, RIGHT);
    }

    void yaw(float direction) {
        float angle = yawSpeed * direction;
        orientation = glm::rotate(orientation, angle, UP);
    }

    void drawAirplane(float deltaTime, ShaderProgram *sp, Mesh *airplaneMesh, GLuint tex0, GLuint tex1) {
        update(deltaTime);

        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M, position);
        M = M * glm::mat4_cast(orientation);

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
        glBindTexture(GL_TEXTURE_2D, tex1);

        glDrawArrays(GL_TRIANGLES, 0, airplaneMesh->vertices.size());

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));
    }
};

#endif // AIRPLANE_H