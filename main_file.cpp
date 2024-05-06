/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <iomanip>
#include <fstream>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ModelLoader.h"
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"

float delta_time = 0;

float speed_x = 0;
float speed_y = 0;
float movement_x = 0;
float aspectRatio = 1;
bool freecam = true;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 200.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 2.0f;
float maxCamSpeed = 10.0f;
float sensitivity = 0.1f;

bool is_w_pressed = false;
bool is_s_pressed = false;
bool is_a_pressed = false;
bool is_d_pressed = false;
bool is_space_pressed = false;
bool is_lctrl_pressed = false;
bool is_esc_pressed = false;

ShaderProgram* sp;


Mesh airplaneMesh; // na górze pliku main_file.cpp


//Odkomentuj, żeby rysować kostkę
 /*
 float* vertices = myCubeVertices;
float* normals = myCubeNormals;
float* texCoords = myCubeTexCoords;
float* colors = myCubeColors;
int vertexCount = myCubeVertexCount;
*/

//Odkomentuj, żeby rysować czajnik
/*
float* vertices = myTeapotVertices;
float* normals = myTeapotVertexNormals;
float* texCoords = myTeapotTexCoords;
float* colors = myTeapotColors;
int vertexCount = myTeapotVertexCount;
*/



//Procedura obsługi błędów
void error_callback(int error, const char* description) {
    fputs(description, stderr);
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    float velocity = cameraSpeed * delta_time;

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT) speed_x = -PI;
        if (key == GLFW_KEY_RIGHT) speed_x = PI;
        if (key == GLFW_KEY_UP) speed_y = PI;
        if (key == GLFW_KEY_DOWN) speed_y = -PI;
        // if (key == GLFW_KEY_W) movement_x = 150;
        // if (key == GLFW_KEY_S) movement_x = -150;    
        if (key == GLFW_KEY_F) freecam = !freecam;
        if (key == GLFW_KEY_W && freecam) is_w_pressed = true;
        if (key == GLFW_KEY_S && freecam) is_s_pressed = true;
        if (key == GLFW_KEY_A && freecam) is_a_pressed = true;
        if (key == GLFW_KEY_D && freecam) is_d_pressed = true;
        if (key == GLFW_KEY_SPACE && freecam) is_space_pressed = true;
        if (key == GLFW_KEY_LEFT_CONTROL && freecam) is_lctrl_pressed = true;

        if (key == GLFW_KEY_ESCAPE) {
            is_esc_pressed = !is_esc_pressed;
            if (!is_esc_pressed) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT) speed_x = 0;
        if (key == GLFW_KEY_RIGHT) speed_x = 0;
        if (key == GLFW_KEY_UP) speed_y = 0;
        if (key == GLFW_KEY_DOWN) speed_y = 0;

        if (key == GLFW_KEY_W && freecam) is_w_pressed = false;
        if (key == GLFW_KEY_S && freecam) is_s_pressed = false;
        if (key == GLFW_KEY_A && freecam) is_a_pressed = false;
        if (key == GLFW_KEY_D && freecam) is_d_pressed = false;
        if (key == GLFW_KEY_SPACE && freecam) is_space_pressed = false;
        if (key == GLFW_KEY_LEFT_CONTROL && freecam) is_lctrl_pressed = false;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (yoffset > 0) {
        cameraSpeed += 0.1f;
    } else if (yoffset < 0) {
        cameraSpeed = std::max(0.1f, cameraSpeed - 0.1f);
    }
    if (cameraSpeed > maxCamSpeed) cameraSpeed = maxCamSpeed;
}

bool firstMouse = true;
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    //float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraFront = glm::normalize(front);
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) return;
    aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
    glClearColor(0.2, 0.2, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

    ModelLoader loader;
    airplaneMesh = loader.loadModel("models/source/F-16.obj");

}

//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

    delete sp;
}

void drawAirplane(const glm::mat4 &M, const glm::mat4 &P, const glm::mat4 &V, const glm::vec4 lp) {

    sp->use();
    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
    glUniform4fv(sp->u("lp"), 1, glm::value_ptr(lp));

    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, airplaneMesh.vertices.data());

    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, airplaneMesh.normals.data());

    //TODO teksturowanie

    glDrawArrays(GL_TRIANGLES, 0, airplaneMesh.vertices.size());

    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
}

void updateCameraPosition() {
    float velocity = cameraSpeed * delta_time;
    if (is_w_pressed) cameraPos += cameraFront * velocity;
    if (is_s_pressed) cameraPos -= cameraFront * velocity;
    if (is_a_pressed) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    if (is_d_pressed) cameraPos += cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    if (is_space_pressed) cameraPos += cameraUp * velocity;
    if (is_lctrl_pressed) cameraPos -= cameraUp * velocity;
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow *window, float angle_x, float angle_y, float given_movement_x) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (freecam) {
        updateCameraPosition();
    } else {
        cameraPos = glm::vec3(0.0f, 0.0f, 200.0f);
        cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::mat4 V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 50.0f, 5000.0f);

    glm::vec4 lp = glm::vec4(0, 0, -6, 1);
    
    glm::mat4 Mplane = glm::mat4(1.0f);
    Mplane = glm::rotate(Mplane, angle_y, glm::vec3(1.0f, 0.0f, 0.0f));
    Mplane = glm::rotate(Mplane, angle_x, glm::vec3(0.0f, 1.0f, 0.0f));
    Mplane = glm::translate(Mplane, glm::vec3(given_movement_x, 0.0f, 0.0f));
    drawAirplane(Mplane, P, V, lp);

    glfwSwapBuffers(window);
}


int main(void)
{
    GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

    glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

    if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
        fprintf(stderr, "Nie można zainicjować GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

    if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
    {
        fprintf(stderr, "Nie można utworzyć okna.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
    glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

    if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
        fprintf(stderr, "Nie można zainicjować GLEW.\n");
        exit(EXIT_FAILURE);
    }

    initOpenGLProgram(window); //Operacje inicjujące

    //Główna pętla
    float angle_x = 0; //Aktualny kąt obrotu obiektu
    float angle_y = 0; //Aktualny kąt obrotu obiektu
    float given_movement_x = 0;
    glfwSetTime(0); //Zeruj timer
    while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
    {
        delta_time += glfwGetTime();
        angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        given_movement_x += movement_x * glfwGetTime();
        glfwSetTime(0); //Zeruj timer
        drawScene(window, angle_x, angle_y, given_movement_x); // Wykonaj procedurę rysującą
        glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
    }

    freeOpenGLProgram(window);

    glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
    glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW

    exit(EXIT_SUCCESS);
}