#include <fstream>
#include "RigidBody.h"
#include "ModelLoader.h"
#include "shaderprogram.h"

void drawTerrain(ShaderProgram *sp, Mesh *TerrainMesh, GLuint tex) {

    glm::mat4 M = glm::mat4(1.0f);
    M = glm::translate(M, glm::vec3(0, 100, 0));

    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 3, GL_FLOAT, false, 0, TerrainMesh->vertices.data());

    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, TerrainMesh->normals.data());

    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, TerrainMesh->texCoords.data());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glDrawArrays(GL_TRIANGLES, 0, TerrainMesh->vertices.size());



    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
    glDisableVertexAttribArray(sp->a("texCoord0"));
}