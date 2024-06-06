#pragma once


#include <GL/glew.h>
#include <glm/glm.hpp>
#include "shaderprogram.h"


void build(GLuint tex, ShaderProgram *sp, glm::mat4 M1);
void build_sufit(GLuint tex, ShaderProgram *sp, glm::mat4 M1);
void build_city(int b_c, GLuint tex[][2], int budynki_komb[][10], float budynki_skala[][10][2], ShaderProgram *sp,int losowa_liczba);