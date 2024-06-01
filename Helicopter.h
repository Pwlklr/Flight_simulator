#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "ModelLoader.h"
#include "shaderprogram.h"
#include "constants.h"



class Helicopter{
public:
    float mainRotorSpeed;
    float tailRotorSpeed;
    glm::vec3 position{};
    glm::quat orientation{};
    glm::vec3 velocity{};
    glm::vec3 angularVelocity{};
    float maxTiltAngle = glm::radians(25.0f); // Maksymalny k¹t wychylenia w radianach
    float stabilizationRate = 0.1f;
    float turn_rate = glm::radians(100.0f);

    Helicopter() :
            mainRotorSpeed(0.0f),
            tailRotorSpeed(0.0f) {}

    void updateRotors(float deltaTime) {
        mainRotorSpeed += 360.0f * deltaTime;
        tailRotorSpeed += 720.0f * deltaTime; 
    }

    void updateHeli(float deltaTime) {
        // Update position
        position += velocity * deltaTime;

        // Update orientation
        glm::quat deltaOrientation = glm::quat(1.0f, angularVelocity * deltaTime);
        orientation = glm::normalize(glm::cross(deltaOrientation, orientation));

        // Clamp orientation to max tilt angle
        glm::vec3 euler = glm::eulerAngles(orientation);
        euler.x = glm::clamp(euler.x, -maxTiltAngle, maxTiltAngle);
        euler.z = glm::clamp(euler.z, -maxTiltAngle, maxTiltAngle);
        orientation = glm::quat(euler);

        angularVelocity = glm::vec3(0.0f);
    }

    void applyInput(char key, float deltaTime) {

        switch (key) {
        case 'w':
            angularVelocity -= glm::vec3(0.0f, 0.0f, turn_rate * deltaTime);
            break;
        case 's':
            angularVelocity += glm::vec3(0.0f, 0.0f, turn_rate * deltaTime);
            break;
        case 'a':
            angularVelocity -= glm::vec3(turn_rate * deltaTime, 0.0f, 0.0f);
            break;
        case 'd':
            angularVelocity += glm::vec3(turn_rate * deltaTime, 0.0f, 0.0f);
            break;
        case 'q':
            angularVelocity += glm::vec3(0.0f, turn_rate * deltaTime, 0.0f);
            break;
        case 'e':
            angularVelocity -= glm::vec3(0.0f, turn_rate * deltaTime, 0.0f);
            break;
        }
    }

    void drawHelicopter(float delta_time, ShaderProgram *sp, Mesh *bodyMesh, GLuint tex0, GLuint tex1) {
        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M, position);
        M = M * glm::mat4_cast(orientation);
        

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
        /*
        // Rysowanie g³ównego œmig³a
        static float rotorAngle = 0.0f;
        rotorAngle += 10.0f * delta_time;
        if (rotorAngle > 2 * PI) rotorAngle -= 2 * PI;

        M = glm::rotate(glm::mat4(1.0f), rotorAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        M = glm::translate(M, glm::vec3(0.0f, 10.0f, 0.0f));
        glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

        glEnableVertexAttribArray(sp->a("vertex"));
        glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, mainRotorMesh->vertices.data());

        glEnableVertexAttribArray(sp->a("normal"));
        glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, mainRotorMesh->normals.data());

        glEnableVertexAttribArray(sp->a("texCoord0"));
        glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, mainRotorMesh->texCoords.data());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1);

        glDrawArrays(GL_TRIANGLES, 0, mainRotorMesh->vertices.size());

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));

        // Rysowanie tylnego œmig³a
        M = glm::rotate(glm::mat4(1.0f), rotorAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        M = glm::translate(M, glm::vec3(0.0f, 5.0f, -30.0f));
        glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

        glEnableVertexAttribArray(sp->a("vertex"));
        glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, tailRotorMesh->vertices.data());

        glEnableVertexAttribArray(sp->a("normal"));
        glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, tailRotorMesh->normals.data());

        glEnableVertexAttribArray(sp->a("texCoord0"));
        glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, tailRotorMesh->texCoords.data());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1);

        glDrawArrays(GL_TRIANGLES, 0, tailRotorMesh->vertices.size());

        glDisableVertexAttribArray(sp->a("vertex"));
        glDisableVertexAttribArray(sp->a("normal"));
        glDisableVertexAttribArray(sp->a("texCoord0"));
        */
    }

    
};