#define GLM_FORCE_RADIANS

#include "constants.h"
#include "lodepng.h"
#include "myCube.h"
#include "shaderprogram.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct parametry {
    float x;
    float y;
    float z;
    float size_x;
    float size_y;
    float size_z;
};

parametry par[18] = {
    // Parametry kolizji
    { -2100, 110, 2100, 2100,50,2100}, 
    { -2100, 400, -10, 2100, 500, 50 }, 
    {-4090,400,2100, 50,500,2100},
    { -2100, 400, 4100, 2100, 500, 50 },
    { 10, 400, 2100, 50, 500, 2100 },
    {-2100,140,75,2100,350,75},
    {-2100,140,4025,2100,350,75},
    {-700,140,3300,725,350,1050},
    {-75,140,2100,75,350,2100},
    {-2050,140,320,750,250,350},
    {-2850,240,480,350,250,650},
    {-2300,240,750,180,150,150},
    {-3150,140,900,500,90,650},
    { -3350, 140, 1850, 300, 110.0f, 450 },
    { -4000, 240, 3600, 200, 110.0f, 500},
    { -4200, 180, 2100, 300, 90.0f, 2100 },
    { -3600, 350, 3750, 200, 110.0f, 150 },
    { -1950, 175, 2830, 200, 300, 150 }

};

void kolizje(int i, glm::vec3 &cameraPos) {
    if (cameraPos.x <= par[i].x + par[i].size_x && cameraPos.x >= par[i].x - par[i].size_x) {
        if (cameraPos.y <= par[i].y + par[i].size_y && cameraPos.y >= par[i].y - par[i].size_y) {
            if (cameraPos.z <= par[i].z + par[i].size_z && cameraPos.z >= par[i].z - par[i].size_z) {
                float dif[6] = { 0, 0, 0, 0, 0, 0 };
                dif[0] = abs(cameraPos.x - (par[i].x + par[i].size_x));
                dif[1] = abs(cameraPos.x - (par[i].x - par[i].size_x));
                dif[2] = abs(cameraPos.y - (par[i].y + par[i].size_y));
                dif[3] = abs(cameraPos.y - (par[i].y - par[i].size_y));
                dif[4] = abs(cameraPos.z - (par[i].z + par[i].size_z));
                dif[5] = abs(cameraPos.z - (par[i].z - par[i].size_z));
                int num = 0;
                for (int j = 1; j < 6; j++) {
                    if (dif[j] < dif[num]) {
                        num = j;
                    }
                }
                switch (num) {
                case 0:
                    cameraPos.x = par[i].x + par[i].size_x;
                    exit(0);
                    break;
                case 1:
                    cameraPos.x = par[i].x - par[i].size_x;
                    exit(0);
                    break;
                case 2:
                    cameraPos.y = par[i].y + par[i].size_y;
                    exit(0);
                    break;
                case 3:
                    cameraPos.y = par[i].y - par[i].size_y;
                    //exit(0);
                    break;
                case 4:
                    cameraPos.z = par[i].z + par[i].size_z;
                    exit(0);
                    break;
                case 5:
                    cameraPos.z = par[i].z - par[i].size_z;
                    exit(0);
                    break;
                }
            }
        }
    }
}

void gravity(glm::vec3 &cameraPos) { // Przekazujemy przez referencjê
    int numElements = sizeof(par) / sizeof(par[0]); // Liczba elementów w tablicy
    for (int i = 0; i < numElements; i++) {
        kolizje(i, cameraPos);
    }
}
