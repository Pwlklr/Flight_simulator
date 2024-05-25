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
#include <stdlib.h>
#include <stdio.h>
#include <map>

#include "ModelLoader.h"
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include "FlightModel.h"


float delta_time = 0; //zmienna globalna określająca czas między klatkami

float speed_x = 0;
float speed_y = 0;
float movement_x = 0;

//zmienna wykorzystywana do zmiany rozmiaru okna
float aspectRatio = 1.77777778;

//zmienne globalne dotyczące kamery
bool freecam = false;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 200.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 1.0f;
float maxCamSpeed = 5.0f;
float mouseSensitivity = 0.09f;

bool firstMouse = true;
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;

//zmienne globalne wykorzystywane w funkcjionalności freecam
bool is_w_pressed = false;
bool is_s_pressed = false;
bool is_a_pressed = false;
bool is_d_pressed = false;
bool is_space_pressed = false;
bool is_lctrl_pressed = false;
bool is_esc_pressed = false;
bool is_lshift_pressed = false;

float throttle = 1.0f, roll = 0.0f;


ShaderProgram* sp;

// Uchwyty tekstur - deklaracje globalne
GLuint tex0;
GLuint tex1;

Airfoil NACA_2412(NACA_2412_data);
Airfoil NACA_0012(NACA_0012_data);

Mesh airplaneMesh; //struktura ModelLoader, deklarować dla wszystkich wczytywanych modeli

std::vector<Wing> wings = {
    Wing({ 0.0f, 0.0f, -2.7f }, 6.96f, 2.50f, &NACA_2412, UP, 0.20f), // left wing
    Wing({ 0.0f, 0.0f, +2.7f }, 6.96f, 2.50f, &NACA_2412, UP, 0.20f), // right wing
    // Wing({ -6.6f, -0.1f, 0.0f }, 3.12f, 2.70f, &NACA_0012, UP, 1.0f), // elevator
    // Wing({ -6.6f, 0.0f, 0.0f }, 5.31f, 3.10f, &NACA_0012, RIGHT, 0.15f) // rudder
};
Engine simpleEngine(50000);


glm::mat3 testinertia(
        40000.0f, 0.0f, 0.0f,
        0.0f, 100000.0f, 0.0f,
        0.0f, 0.0f, 200000.0f);

Airplane mainAirplane(1000.0f, simpleEngine, testinertia, wings);




GLuint readTexture(const char *filename) {
    GLuint tex;
    glActiveTexture(GL_TEXTURE0);
    // Wczytanie do pamięci komputera
    std::vector<unsigned char> image; // Alokuj wektor do wczytania obrazka
    unsigned width, height; // Zmienne do których wczytamy wymiary obrazka
    // Wczytaj obrazek
    unsigned error = lodepng::decode(image, width, height, filename);
    // Import do pamięci karty graficznej
    glGenTextures(1, &tex); // Zainicjuj jeden uchwyt
    glBindTexture(GL_TEXTURE_2D, tex); // Uaktywnij uchwyt
    // Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)image.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}



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
        if (key == GLFW_KEY_W) is_w_pressed = true;
        if (key == GLFW_KEY_S) is_s_pressed = true;
        if (key == GLFW_KEY_A) is_a_pressed = true;
        if (key == GLFW_KEY_D) is_d_pressed = true;
        if (key == GLFW_KEY_SPACE) is_space_pressed = true;
        if (key == GLFW_KEY_LEFT_CONTROL) is_lctrl_pressed = true;
        if (key == GLFW_KEY_LEFT_SHIFT) is_lshift_pressed = true;

        // if (key == GLFW_KEY_A && !freecam) mainAirplane.roll(-1);
        // if (key == GLFW_KEY_D && !freecam) mainAirplane.roll(1);
        // if (key == GLFW_KEY_LEFT_SHIFT && !freecam) mainAirplane.pitch(1);
        // if (key == GLFW_KEY_LEFT_CONTROL && !freecam) mainAirplane.pitch(-1);
        // if (key == GLFW_KEY_Q && !freecam) mainAirplane.yaw(-1);
        // if (key == GLFW_KEY_E && !freecam) mainAirplane.yaw(1);

        if (key == GLFW_KEY_ESCAPE) {
            is_esc_pressed = !is_esc_pressed;
            if (!is_esc_pressed) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT) speed_x = 0;
        if (key == GLFW_KEY_RIGHT) speed_x = 0;
        if (key == GLFW_KEY_UP) speed_y = 0;
        if (key == GLFW_KEY_DOWN) speed_y = 0;

        if (key == GLFW_KEY_W) is_w_pressed = false;
        if (key == GLFW_KEY_S) is_s_pressed = false;
        if (key == GLFW_KEY_A) is_a_pressed = false;
        if (key == GLFW_KEY_D) is_d_pressed = false;
        if (key == GLFW_KEY_SPACE) is_space_pressed = false;
        if (key == GLFW_KEY_LEFT_CONTROL) is_lctrl_pressed = false;
        if (key == GLFW_KEY_LEFT_SHIFT) is_lshift_pressed = false;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (yoffset > 0) {
        cameraSpeed += 0.05f;
    } else if (yoffset < 0) {
        cameraSpeed = std::max(0.1f, cameraSpeed - 0.1f);
    }
    if (cameraSpeed > maxCamSpeed) cameraSpeed = maxCamSpeed;
}

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
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

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

//procedura do obsługi ruchu kamerą
void updateCameraPosition() {
    float velocity = cameraSpeed * delta_time;
    if (is_w_pressed) cameraPos += cameraFront * velocity;
    if (is_s_pressed) cameraPos -= cameraFront * velocity;
    if (is_a_pressed) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    if (is_d_pressed) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    if (is_space_pressed) cameraPos += cameraUp * velocity;
    if (is_lctrl_pressed) cameraPos -= cameraUp * velocity;
}

// Procedura inicjująca
void initOpenGLProgram(GLFWwindow *window) {
    //************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
    glClearColor(0.2, 0.2, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl"); 

    tex0 = readTexture("F-16_Airframe_BaseColorFinalized.png");
    tex1 = readTexture("/models/textures/sky.png");

    ModelLoader loader;
    airplaneMesh = loader.loadModel("models/source/F-16.obj");
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4 V;

    if (freecam) {
        updateCameraPosition();
        V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    } else {
        // Kod śledzenia samolotu
        glm::vec3 airplanePos = mainAirplane.position;
        // Umieść kamerę za samolotem i nieco powyżej, patrząc w jego kierunku
        V = glm::lookAt(airplanePos + glm::vec3(-200.0f, 20.0f, 0.0f), airplanePos, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 50.0f, 5000.0f);

    glm::vec4 lp = glm::vec4(0, 6, 0, 1);

    if (is_w_pressed) {
        throttle += 0.001;
        throttle = glm::clamp(throttle, 0.0f, 1.0f);
        mainAirplane.throttle = throttle;
    }
    else if (is_s_pressed) {
        throttle -= 0.001;
        throttle = glm::clamp(throttle, 0.0f, 1.0f);
        mainAirplane.throttle = throttle;
    }
    if (is_a_pressed) {
        roll = -1.0f;
        mainAirplane.roll = roll;
    } else if (is_d_pressed) {
        roll = 1.0f;
        mainAirplane.roll = roll;
    }
    if (is_lctrl_pressed) {
        roll = -1.0f;
        mainAirplane.pitch = pitch;
    } else if (is_lshift_pressed) {
        roll = 1.0f;
        mainAirplane.pitch = pitch;
    }

    sp->use();
    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniform4fv(sp->u("lp"), 1, glm::value_ptr(lp));
    glUniform1i(sp->u("textureMap0"), 0);
    glUniform1i(sp->u("textureMap1"), 1);
    glUniform1f(sp->u("lightIntensity"), 1.25f);
    

    mainAirplane.drawAirplane(delta_time, sp, &airplaneMesh, tex0, tex1);

    glfwSwapBuffers(window);
}

// Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow *window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

    delete sp;
}


int main(void)
{
    GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

    glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

    if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
        fprintf(stderr, "Nie można zainicjować GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(1280, 720, "OpenGL", NULL, NULL); // Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

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
        drawScene(window); // Wykonaj procedurę rysującą
        glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
    }

    freeOpenGLProgram(window);

    glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
    glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW

    exit(EXIT_SUCCESS);
}