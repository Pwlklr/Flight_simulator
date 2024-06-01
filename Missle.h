#pragma once
#ifndef MISSLE_H
#define MISSLE_H

#include "shaderprogram.h"
#include "ModelLoader.h"
#include "FlightModel.h"
#include "RigidBody.h"
#include "constants.h"

class Missile : public RigidBody {
public:
    Engine engine;
    float flight_time, max_flight_time;

    Missile(float mass, const Engine &engine, const glm::mat3 &inertia, glm::vec3 initial_position, glm::quat initial_orientation, glm::vec3 initial_velocity, float max_flight_time = 1000.0f) :
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

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));
    }
};

#endif