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
#include <random>
#include <ctime>

#include "ModelLoader.h"
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include "FlightModel.h"
#include "builds.h"
#include "Colission.h"
#include "Terrain.h"
#include "builds.h"

// <<<<<<< Miasto_Generator

// //#include "building1.h"


// namespace inertia {

// template <typename T>
// constexpr inline T sq(T x) {
//     return x * x;
// }

// // mass element used for inertia tensor calculation
// struct Element {
//     glm::vec3 size;
//     glm::vec3 position; // position in design coordinates
//     glm::vec3 inertia; // moment of inertia
//     glm::vec3 offset; // offset from center of gravity
//     float mass;
//     float volume() const { return size.x * size.y * size.z; }
// };

// // cuboid moment of inertia
// inline glm::vec3 cuboid(float mass, const glm::vec3 &size) {
//     float x = size.x, y = size.y, z = size.z;
//     return glm::vec3(sq(y) + sq(z), sq(x) + sq(z), sq(x) + sq(y)) * (1.0f / 12.0f) * mass;
// }

// // helper function for the creation of a cuboid mass element
// inline Element cube(const glm::vec3 &position, const glm::vec3 &size, float mass = 0.0f) {
//     glm::vec3 inertia = cuboid(mass, size);
//     return { size, position, inertia, position, mass };
// }
// // calculate inertia tensor for a collection of connected masses
// inline glm::mat3 tensor(std::vector<Element> &elements, bool precomputed_offset = false, glm::vec3 *cg = nullptr) {
//     float Ixx = 0, Iyy = 0, Izz = 0;
//     float Ixy = 0, Ixz = 0, Iyz = 0;

//     float mass = 0;
//     glm::vec3 moment_of_inertia(0.0f);

//     for (const auto &element : elements) {
//         mass += element.mass;
//         moment_of_inertia += element.mass * element.position;
//     }

//     const auto center_of_gravity = moment_of_inertia / mass;

//     for (auto &element : elements) {
//         if (!precomputed_offset) {
//             element.offset = element.position - center_of_gravity;
//         } else {
//             element.offset = element.position;
//         }

//         const auto offset = element.offset;

//         Ixx += element.inertia.x + element.mass * (sq(offset.y) + sq(offset.z));
//         Iyy += element.inertia.y + element.mass * (sq(offset.z) + sq(offset.x));
//         Izz += element.inertia.z + element.mass * (sq(offset.x) + sq(offset.y));
//         Ixy += element.mass * (offset.x * offset.y);
//         Ixz += element.mass * (offset.x * offset.z);
//         Iyz += element.mass * (offset.y * offset.z);
//     }

//     if (cg != nullptr) {
//         *cg = center_of_gravity;
//     }

//     // clang-format off
//   return {
//       Ixx, -Ixy, -Ixz, 
//       -Ixy, Iyy, -Iyz, 
//       -Ixz, -Iyz, Izz
//   };
//     // clang-format on
// }

// }
// =======

// >>>>>>> master


float delta_time = 0; //zmienna globalna określająca czas między klatkami

//zmienna wykorzystywana do zmiany rozmiaru okna
float aspectRatio = 1.77777778;

//zmienne globalne dotyczące kamery
bool freecam = false;
bool plane = false; 
glm::vec3 cameraPos = glm::vec3(1000.0f, 300.0f, 1000.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 150.0f;
float maxCamSpeed = 500.0f;
float mouseSensitivity = 0.09f;

bool firstMouse = true;
float lastX = 400, lastY = 300;
float yawx = -90.0f, pitchx = 0.0f;

//zmienne globalne wykorzystywane w funkcjionalności freecam
bool is_w_pressed = false;
bool is_s_pressed = false;
bool is_a_pressed = false;
bool is_d_pressed = false;
bool is_q_pressed = false;
bool is_e_pressed = false;
bool is_space_pressed = false;
bool is_lctrl_pressed = false;
bool is_lshift_pressed = false;
bool is_esc_pressed = false;


ShaderProgram* sp;

// Uchwyty tekstur - deklaracje globalne
GLuint airplaneTex;
GLuint missileTex;
GLuint heliTex;
GLuint terrainTex;
GLuint explosionTex;
const int b_c = 10; // building_count - ile roznych budynkow jest
// Uchwyty budynki
GLuint tex[11][2]; // uchwyty na budynki



//std::vector<Mesh> sky_scrapers_mesh(4);
Mesh airplaneMesh; //struktura ModelLoader, deklarować dla wszystkich wczytywanych modeli
Mesh missleMesh;
Mesh helicopterMesh;
Mesh terrainMesh;
Mesh rotorMesh;
int losowa_liczba;
int budynki_komb[b_c][b_c]; // uklad budynkow jakie i gdzie zostana narysowane
float budynki_skala[b_c][b_c][2];
const float mass = 10000.0f;
const float thrust = 125000.0f;
float throttle = 1.0f;


const float wing_offset = 0.0f;
const float tail_offset = -6.6f;

Airfoil NACA_2412(NACA_2412_data);
Airfoil NACA_0012(NACA_0012_data);

Engine mainEngine(thrust);

std::vector<Wing> wings = {
    // Przykładowe skrzydło
    Wing({ wing_offset, 0.0f, -5.7f }, 10.0f, 2.5f, &NACA_2412, UP, 0.20f), // left wing
    Wing({ wing_offset, 0.0f, +5.7f }, 10.0f, 2.50f, &NACA_2412, UP, 0.20f), // right wing
    Wing({ tail_offset, -0.1f, 0.0f }, 6.54f, 2.70f, &NACA_0012, UP, 1.0f), // elevator
    Wing({ tail_offset, 0.0f, 0.0f }, 5.31f, 3.10f, &NACA_0012, RIGHT, 0.15f), // rudder
    // Dodaj więcej skrzydeł według potrzeb
};

std::vector<Wing> wings2 = {
    Wing({ wing_offset - 1.5f, 0.0f, -2.0f }, 3.80f, 1.26f, &NACA_0012, UP, 1.0f), // left aileron
    Wing({ wing_offset - 1.5f, 0.0f, 2.0f }, 3.80f, 1.26f, &NACA_0012, UP, 1.0f), // right aileron
    Wing({ tail_offset, -0.1f, 0.0f }, 6.54f, 2.70f, &NACA_0012, UP, 1.0f), // elevator
    Wing({ tail_offset, 0.0f, 0.0f }, 5.31f, 3.10f, &NACA_0012, RIGHT, 1.0f), // rudder
    Wing({ wing_offset, 0.0f, -2.7f }, 15.0f, 2.5f, &NACA_2412, UP, 0.2f), // left wing
    Wing({ wing_offset, 0.0f, +2.7f }, 15.0f, 2.50f, &NACA_2412, UP, 0.2f), // right wing
};

std::vector<Wing> wings3 = {
    Wing({ wing_offset, 0.0f, -2.7f }, 6.96f, 2.70f, &NACA_2412, UP, 0.20f), // left wing
    Wing({ wing_offset, 0.0f, +2.7f }, 6.96f, 2.70f, &NACA_2412, UP, 0.20f), // right wing
    Wing({ tail_offset, -0.1f, 0.0f }, 6.54f, 2.70f, &NACA_0012, UP, 1.0f), // elevator
    Wing({ tail_offset, 0.0f, 0.0f }, 5.31f, 3.10f, &NACA_0012, RIGHT, 0.15f), // rudder
};

glm::mat3 giveninertia = {
    48531.0f, -1320.0f, 0.0f,
    -1320.0f, 256608.0f, 0.0f,
    0.0f, 0.0f, 211333.0f
};

glm::mat3 testinertia = {
    10000.0f, -1320.0f, 0.0f,
    -1320.0f, 20000.0f, 0.0f,
    0.0f, 0.0f, 30000.0f
};

glm::vec3 initialPos(-2000.0f, 100.0f, 1000.0f);

Airplane mainAirplane(mass, mainEngine, giveninertia, wings2, initialPos);

Airplane testAirplane(mass, mainEngine, testinertia, wings, glm::vec3(0.0f, 1000.0f, 0.0f));

Engine heliEngine(15500.0f);
float heliMass = 750.0f;
Helicopter mainHelicopter(heliMass, heliEngine, testinertia, initialPos);


GLuint read_texture_building(const char *filename) {
    GLuint tex;
    glActiveTexture(GL_TEXTURE0);
    // wczytanie do pamięci komputera
    std::vector<unsigned char> image;
    unsigned width, height;
    // wczytujemy obrazek do pamieci komputera
    unsigned error = lodepng::decode(image, width, height, filename);

    if (error) {
        std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
        return 0; // Zwróć 0, aby wskazać błąd wczytywania tekstury
    }

    // GLuint tex[10]; 10,&tex //10 uchwytow
    // import do pamieci graficznej
    glGenTextures(1, &tex); // jeden uchwyt &tex - tablica zmiennych przechowujaca nowe uchwyty
    glBindTexture(GL_TEXTURE_2D, tex); // uaktywnij uchwyt
    // kopiuje obrazek z pamieci komputera do pamieci gpu reprezentowanej przez uchwyt
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, // 24 bitowy - GL_RGB
            GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)image.data()); // image.data wskaznik na dane tekstury
    // wybieramy algorytm probkowania kolorow
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    return tex;
}

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

        // if (key == GLFW_KEY_W) movement_x = 150;
        // if (key == GLFW_KEY_S) movement_x = -150;
        if (key == GLFW_KEY_F) freecam = !freecam;
        if (key == GLFW_KEY_W) is_w_pressed = true;
        if (key == GLFW_KEY_S) is_s_pressed = true;
        if (key == GLFW_KEY_A) is_a_pressed = true;
        if (key == GLFW_KEY_D) is_d_pressed = true;
        if (key == GLFW_KEY_Q) is_q_pressed = true;
        if (key == GLFW_KEY_E) is_e_pressed = true;
        if (key == GLFW_KEY_X) {
            if (plane && !freecam) {
                mainAirplane.fireMissile();
            } else if (!freecam) {
                mainHelicopter.fireMissile();
            }
        }
        if (key == GLFW_KEY_SPACE) is_space_pressed = true;
        if (key == GLFW_KEY_LEFT_CONTROL) is_lctrl_pressed = true;
        if (key == GLFW_KEY_LEFT_SHIFT) is_lshift_pressed = true;

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
        if (key == GLFW_KEY_W) is_w_pressed = false;
        if (key == GLFW_KEY_S) is_s_pressed = false;
        if (key == GLFW_KEY_A) is_a_pressed = false;
        if (key == GLFW_KEY_D) is_d_pressed = false;
        if (key == GLFW_KEY_Q) is_q_pressed = false;
        if (key == GLFW_KEY_E) is_e_pressed = false;
        if (key == GLFW_KEY_SPACE) is_space_pressed = false;
        if (key == GLFW_KEY_LEFT_CONTROL) is_lctrl_pressed = false;
        if (key == GLFW_KEY_LEFT_SHIFT) is_lshift_pressed = false;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (yoffset > 0) {
        cameraSpeed += 0.75f;
    } else if (yoffset < 0) {
        cameraSpeed = std::max(0.1f, cameraSpeed - 0.75f);
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

    yawx += xoffset;
    pitchx += yoffset;

    if (pitchx > 89.0f)  pitchx = 89.0f;
    if (pitchx < -89.0f) pitchx = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(pitchx)) * cos(glm::radians(yawx));
    front.y = sin(glm::radians(pitchx));
    front.z = cos(glm::radians(pitchx)) * sin(glm::radians(yawx));
    cameraFront = glm::normalize(front);
}

/*
void drawTerrain(ShaderProgram *sp, Mesh *bodyMesh, GLuint tex0) {
    glm::mat4 M = glm::mat4(1.0f);
    // M = glm::scale(M, glm::vec3(10.0f, 10.0f, 10.0f));

    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, bodyMesh->vertices.data());

    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, bodyMesh->normals.data());

    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, bodyMesh->texCoords.data());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);

    glDrawArrays(GL_TRIANGLES, 0, bodyMesh->vertices.size());

    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
    glDisableVertexAttribArray(sp->a("texCoord0"));
}
*/
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl"); 

    airplaneTex = readTexture("F-16_Airframe_BaseColorFinalized.png");
    missileTex = readTexture("rocket.png");
    heliTex = readTexture("heli_outside_skin_2.tga.png");
    terrainTex = readTexture("colormap.png");
    explosionTex = readTexture("explosion.png");

    ModelLoader loader;
    airplaneMesh = loader.loadModel("models/source/F-16.obj");
    missleMesh = loader.loadModel("models/source/rocket.3ds");
    helicopterMesh = loader.loadModel("models/source/Large_Helicopter2.blend");
    terrainMesh = loader.loadModel("models/source/lowres-model.obj");
    rotorMesh = loader.loadModel("models/source/rotor.obj");

    // wczytywanie i import obrazka
    tex[0][0] = read_texture_building("images2/building0.png");
    tex[0][1] = read_texture_building("models/textures/dach0.png");
    tex[1][0] = read_texture_building("images2/building1.png");
    tex[1][1] = read_texture_building("models/textures/dach1.png");
    tex[2][0] = read_texture_building("images2/building2.png");
    tex[2][1] = read_texture_building("models/textures/dach2.png");
    tex[3][0] = read_texture_building("images2/building3.png");
    tex[3][1] = read_texture_building("models/textures/dach3.png");
    tex[4][0] = read_texture_building("images2/building4.png");
    tex[4][1] = read_texture_building("models/textures/dach4.png");
    tex[5][0] = read_texture_building("images2/building5.png");
    tex[5][1] = read_texture_building("models/textures/dach5.png");
    tex[6][0] = read_texture_building("images2/building6.png");
    tex[6][1] = read_texture_building("models/textures/dach6.png");
    tex[7][0] = read_texture_building("images2/building7.png");
    tex[7][1] = read_texture_building("models/textures/dach7.png");
    tex[8][0] = read_texture_building("images2/building8.png");
    tex[8][1] = read_texture_building("models/textures/dach8.png");
    tex[9][0] = read_texture_building("images2/building9.png");
    tex[9][1] = read_texture_building("models/textures/dach9.png");
    tex[10][0] = read_texture_building("models/textures/dach2.png");
    std::random_device rd1;
    std::mt19937 gen1(rd1());
    std::uniform_int_distribution<> dis1(10000, 99999);
    losowa_liczba = dis1(gen1);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, b_c - 1);
    std::uniform_real_distribution<> distrib_real(0.8, 1.2);
    for (int i = 0; i < b_c; i++) {
        for (int j = 0; j < b_c; j++) {
            budynki_komb[i][j] = distrib(gen);
            budynki_skala[i][j][0] = distrib_real(gen);
            budynki_skala[i][j][1] = distrib_real(gen);
        }
    }
}

glm::vec4 sunPosition = glm::vec4(1000, 10000, 0, 1);
glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec4 secondLightPosition = glm::vec4(100, 1000, 0, 1);
glm::vec3 secondLightColor = glm::vec3(1.0f, 0.0f, 0.0f);
float ambientStrength = 0.1f;
float secondIntensity = 10.0f;
    

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 V;
    glm::vec3 viewPos;

    if (freecam) {
        updateCameraPosition();
        V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        viewPos = cameraPos;
        //gravity(cameraPos);ZMIANA UPDATE CHANGE 
    } else if (plane) {
        gravity(mainAirplane.position);
        glm::vec3 planeCamPos = mainAirplane.position + mainAirplane.orientation * glm::vec3(-200.0f, 10.0f, 0.0f);
        glm::vec3 planeCamlookAtPoint = mainAirplane.position;
        glm::vec3 planeCamUp = mainAirplane.orientation * glm::vec3(0.0f, 1.0f, 0.0f);
        V = glm::lookAt(planeCamPos, planeCamlookAtPoint, planeCamUp);
        viewPos = cameraPos;
    } else {
        gravity(mainHelicopter.position);
        glm::vec3 planeCamPos = mainHelicopter.position + mainHelicopter.orientation * glm::vec3(-300.0f, 15.0f, 0.0f);
        glm::vec3 planeCamlookAtPoint = mainHelicopter.position;
        glm::vec3 planeCamUp = mainHelicopter.orientation * glm::vec3(0.0f, 1.0f, 0.0f);
        V = glm::lookAt(planeCamPos, planeCamlookAtPoint, planeCamUp);
        viewPos = cameraPos;
    }

    if (!freecam) {
        if (is_w_pressed) {
            throttle += 0.001;
            throttle = glm::clamp(throttle, 0.0f, 1.0f);
            mainAirplane.control_throttle = throttle;
            mainHelicopter.throttle = throttle;

        } else if (is_s_pressed) {
            throttle -= 0.001;
            throttle = glm::clamp(throttle, 0.0f, 1.0f);
            mainAirplane.control_throttle = throttle;
            mainHelicopter.throttle = throttle;
        }

        if (is_a_pressed) {
            mainAirplane.aileron = -1;
            mainHelicopter.applyInput('a', delta_time);
        } else if (is_d_pressed) {
            mainAirplane.aileron = 1;
            mainHelicopter.applyInput('d', delta_time);
        } else {
            mainAirplane.aileron = 0;
        }
        if (is_q_pressed) {
            mainAirplane.rudder = -1;
            mainHelicopter.applyInput('q', delta_time);
        } else if (is_e_pressed) {
            mainAirplane.rudder = 1;
            mainHelicopter.applyInput('e', delta_time);
        } else {
            mainAirplane.rudder = 0;
        }

        if (is_lshift_pressed) {
            mainAirplane.elevator = 1;
            mainHelicopter.applyInput('w', delta_time);
        } else if (is_lctrl_pressed) {
            mainAirplane.elevator = -1;
            mainHelicopter.applyInput('s', delta_time);
        } else {
            mainAirplane.elevator = 0;
        }
    }

    glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 50.0f, 5000.0f);

    secondLightPosition = glm::vec4(mainHelicopter.position + glm::vec3(-10.0f, 10.0f, 0.0f), 1.0f);

    sp->use();
    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniform4fv(sp->u("sunPosition"), 1, glm::value_ptr(sunPosition));
    glUniform3fv(sp->u("sunColor"), 1, glm::value_ptr(sunColor));
    glUniform4fv(sp->u("secondLightPosition"), 1, glm::value_ptr(secondLightPosition));
    glUniform3fv(sp->u("secondLightColor"), 1, glm::value_ptr(secondLightColor));
    glUniform1f(sp->u("ambientStrength"), ambientStrength);
    glUniform1f(sp->u("secondLightIntensity"), secondIntensity);
    glUniform3fv(sp->u("viewPos"), 1, glm::value_ptr(viewPos));
    glUniform1i(sp->u("textureMap0"), 0);
    glUniform1i(sp->u("textureMap1"), 1);
    glUniform1f(sp->u("lightIntensity"), 1.25f);

    
   // glm::mat4 Mplane = glm::mat4(1.0f);
   // Mplane = glm::rotate(Mplane, angle_y, glm::vec3(1.0f, 0.0f, 0.0f));
  //  Mplane = glm::rotate(Mplane, angle_x, glm::vec3(0.0f, 1.0f, 0.0f));
    //Mplane = glm::translate(Mplane, glm::vec3(given_movement_x, 0.0f, 0.0f));
    // drawAirplane(Mplane);
    
    //z pliku flight model.h
   // mainAirplane.drawAirplane(delta_time, sp, &airplaneMesh, tex0, tex1);
    //z pliku buildings.cpp
    build_city(b_c, tex, budynki_komb, budynki_skala, sp,losowa_liczba);
    


    drawTerrain(sp, &terrainMesh, terrainTex);
    // z pliku buildings.cpp
    build_city(b_c, tex, budynki_komb, budynki_skala, sp, losowa_liczba);
    // testAirplane.drawAirplane(0, sp, &airplaneMesh, &missleMesh, airplaneTex, airplaneTex);


    if (plane && !freecam) {
        mainAirplane.update(delta_time);
        mainAirplane.drawAirplane(delta_time, sp, &airplaneMesh, &missleMesh, airplaneTex, missileTex);
    } else if (!plane) {
        mainHelicopter.updateHeli(delta_time);
        mainHelicopter.drawHelicopter(delta_time, V, sp, &helicopterMesh, &rotorMesh, &missleMesh, heliTex, missileTex, explosionTex);
    }


    glfwSwapBuffers(window);
}

// Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow *window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
    for (int i = 0; i < 10; ++i) {
        glDeleteTextures(2, tex[i]);
    }
    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("texCoord0"));
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
    float prev_time = glfwGetTime();
    glfwSetTime(0); //Zeruj timer
    while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
    {
        float current_time = glfwGetTime(); 
        delta_time = current_time - prev_time;
        prev_time = current_time;

        //glfwSetTime(0); //Zeruj timer
        drawScene(window); // Wykonaj procedurę rysującą
        glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
    }

    freeOpenGLProgram(window);

    glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
    glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW

    exit(EXIT_SUCCESS);
}
