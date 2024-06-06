

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

#include <RigidBody.h>
#include <ModelLoader.h>
#include <building1.h>
#include <shaderprogram.h>

void build(GLuint tex, ShaderProgram *sp, glm::mat4 M1) {
    GLuint tex2 = tex;
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M1));
    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verts1);

    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex2); // przypisuje uchwyt tex2 do jednostki teksturujacej nr 0

    glUniform1i(sp->u("tex"), 0);
    glDrawArrays(GL_TRIANGLES, 0, building_VertexCount1);

}
void build_sufit(GLuint tex, ShaderProgram *sp, glm::mat4 M1) {
    GLuint tex3 = tex;
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M1));
    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verts_sufit1);

    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords_sufit1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex3); // przypisuje uchwyt tex2 do jednostki teksturujacej nr 0

    glUniform1i(sp->u("tex"), 0);
    glDrawArrays(GL_TRIANGLES, 0, sufit_VertexCount1);
}


void build_city(int b_c, GLuint tex[][2], int budynki_komb[][10], float budynki_skala[][10][2], ShaderProgram *sp,int losowa_liczba) {
    int srodkowe = 3;
    
    // Generowanie losowej liczby 5-cyfrowej
  
    glm::mat4 M1 = glm::mat4(1.0f);
    M1 = glm::scale(M1, glm::vec3(2.0f, 2.0f, 2.0f));
    for (int i = 0; i < b_c; i++) {
        for (int j = 0; j < b_c; j++) {
            M1 = glm::scale(M1, glm::vec3(budynki_skala[i][j][0], 1.0f, budynki_skala[i][j][1]));
            if (i <= 3 || i >= 6 || j < srodkowe || j >= b_c - srodkowe) {


                if (j < srodkowe && i > 3 && i < 6) {
                    build(tex[budynki_komb[i][j]][0], sp, M1);
                    // jesli to jest ten co ma byc wiekszy translacja wyzej i budowa TRANSLACJA potem nizej zeby wrocic PO SUFICIE aktualnie kazdy budynek ma wysokosc 40 stad translacia o 40 w gore
                    //  zbuduje mi dwa sufity Ale nie wplywa to na wyglad gdyz tego ponizej nie widac
                    // musi byc +40 dajace wysokosc 80 bo gdy np bedzie 30 to dwa na siebie beda najezdzac skutkujac: wysokoscia 70, MIGANIEM czesci wspolnej dwoch budynkow najpierw laduje jedna klatke potem druga
                    if (budynki_komb[i][j] == 6 || budynki_komb[i][j] == 7) {
                        M1 = glm::translate(M1, glm::vec3(0.0f, 40.0f, 0.0f));
                        build(tex[budynki_komb[i][j]][0], sp, M1);
                        build_sufit(tex[budynki_komb[i][j]][1], sp, M1);
                        M1 = glm::translate(M1, glm::vec3(0.0f, -40.0f, 0.0f));
                    }
                    build_sufit(tex[budynki_komb[i][j]][1], sp, M1);
                    M1 = glm::translate(M1, glm::vec3(100.0f, 0.0f, 0.0f));
                }
                if (j >= b_c - srodkowe && i > 3 && i < 6) {//TO SPRAWIA ZE SA DALEKO 
                   // M1 = glm::translate(M1, glm::vec3(100.0f * (j - srodkowe), 0.0f, 0.0f));//TO SPRAWIA ZE SA DALEKO jak odkomentuje beda daleko
                    build(tex[budynki_komb[i][j]][0], sp, M1);
                    // jesli to jest ten co ma byc wiekszy translacja wyzej i budowa potem TRANSLACJA nizej zeby wrocic PO SUFICIE aktualnie kazdy budynek ma wysokosc 40 stad translacia o 40 w gore
                    // zbuduje mi dwa sufity Ale nie wplywa to na wyglad gdyz tego ponizej nie widac
                    if (budynki_komb[i][j] == 2 || budynki_komb[i][j] == 3) {
                        M1 = glm::translate(M1, glm::vec3(0.0f, 40.0f, 0.0f));
                        build(tex[budynki_komb[i][j]][0], sp, M1);
                        build_sufit(tex[budynki_komb[i][j]][1], sp, M1);
                        M1 = glm::translate(M1, glm::vec3(0.0f, -40.0f, 0.0f));
                    }
                    build_sufit(tex[budynki_komb[i][j]][1], sp, M1);
                 //   M1 = glm::translate(M1, glm::vec3(-100.0f * (j - srodkowe), 0.0f, 0.0f));//TO SPRAWIA ZE SA DALEKO  jak odkomentuje beda daleko
                    M1 = glm::translate(M1, glm::vec3(100.0f, 0.0f, 0.0f));
                }
                if (i <= 3 || i >= 6) {

                    build(tex[budynki_komb[i][j]][0], sp, M1);
                   
                    // jesli to jest ten co ma byc wiekszy translacja wyzej i budowa potem TRANSLACJA nizej zeby wrocic PO SUFICIE aktualnie kazdy budynek ma wysokosc 40 stad translacia o 40 w gore
                    // zbuduje mi dwa sufity Ale nie wplywa to na wyglad gdyz tego ponizej nie widac
                    if (budynki_komb[i][j] == 4 || budynki_komb[i][j] == 5) {
                        M1 = glm::translate(M1, glm::vec3(0.0f, 40.0f, 0.0f));
                        build(tex[budynki_komb[i][j]][0], sp, M1);
                       
                        build_sufit(tex[budynki_komb[i][j]][1], sp, M1);
                        
                        M1 = glm::translate(M1, glm::vec3(0.0f, -40.0f, 0.0f));
                    }
                    build_sufit(tex[budynki_komb[i][j]][1], sp, M1);
                    M1 = glm::translate(M1, glm::vec3(100.0f, 0.0f, 0.0f));
                }
               
            }
            // TUTAJ BARDZIEJ SKOMPLIKOWANE JEST TO MNIEJ WIECEJ SRODEK CALEGO MIASTA
            if (j >= srodkowe + 1 && j < b_c - srodkowe -1  && i > 3 && i < 6 ) 
            {
                int nr=0;
                M1 = glm::translate(M1, glm::vec3(50.0f, 0.0f, 0.0f));
                glm::mat4 M_POM = M1;
                if (losowa_liczba % 4 == 0)
                {
                    nr = 3;
                    M1 = glm::translate(M1, glm::vec3(0.0f, 10.0f, 0.0f));//10 = (40*1.5/2) - 20 NIE USUWAĆ POD ŻADNYM POZOREM
                    M1 = glm::scale(M1, glm::vec3(1.4f, 1.5f, 1.25f));
                }
                if (losowa_liczba % 4 == 1)
                {
                    nr = 4;
                    M1 = glm::scale(M1, glm::vec3(1.3f, 1.0f, 1.2f));
                } 
                 if (losowa_liczba % 4 == 2)
                {
                    nr = 8;
                     M1 = glm::scale(M1, glm::vec3(1.1f, 1.0f, 1.2f));
                }
                 if (losowa_liczba % 4 == 3)
                {
                    nr = 9;   
                    M1 = glm::scale(M1, glm::vec3(0.8f, 1.0f, 0.9f));
                }
                losowa_liczba = losowa_liczba / 10;
                M1 = glm::translate(M1, glm::vec3(0.0f, 34.0f, 0.0f));
                //potrzebna translacja w gore aby nie byly pod ziemia (34 = (40*2.7/2) - 20)//NIE USUWAĆ
                M1 = glm::scale(M1, glm::vec3(1.5f, 2.7f, 1.5f));
                
                build(tex[nr][0], sp, M1);
                build_sufit(tex[nr][1], sp, M1);
                M1 = glm::translate(M1, glm::vec3(0.0f, 30.0f, 0.0f));
                M1 = glm::scale(M1, glm::vec3(0.5f, 0.5f, 0.5f));
                build(tex[nr][0], sp, M1);
                build_sufit(tex[nr][1], sp, M1);
                M1 = glm::translate(M1, glm::vec3(0.0f, 30.0f, 0.0f));
                M1 = glm::scale(M1, glm::vec3(0.5f, 0.5f, 0.5f));
                build(tex[nr][0], sp, M1);
                build_sufit(tex[nr][1], sp, M1);
                M1 = glm::translate(M1, glm::vec3(0.0f, 30.0f, 0.0f));
                M1 = glm::scale(M1, glm::vec3(0.5f, 0.5f, 0.5f));
                build(tex[nr][0], sp, M1);
                build_sufit(tex[nr][1], sp, M1);
                if (nr == 3)
                {
                    M1 = glm::translate(M1, glm::vec3(0.0f, 30.0f, 0.0f));
                    M1 = glm::scale(M1, glm::vec3(0.5f, 0.5f, 0.5f));
                    build(tex[nr][0], sp, M1);
                    build_sufit(tex[nr][1], sp, M1);
                    M1 = glm::translate(M1, glm::vec3(0.0f, 30.0f, 0.0f));
                    M1 = glm::scale(M1, glm::vec3(0.5f, 0.5f, 0.5f));
                    build(tex[nr][0], sp, M1);
                    build_sufit(tex[nr][1], sp, M1);
                }
                M1 = M_POM;
                M1 = glm::translate(M1, glm::vec3(100.0f, 0.0f, 0.0f));
                
            }
        }
        M1 = glm::mat4(1.0f);
        M1 = glm::scale(M1, glm::vec3(2.0f, 2.0f, 2.0f));
        M1 = glm::translate(M1, glm::vec3(0.0f, 0.0f, 100.0f * (1 + i)));
    }
}
