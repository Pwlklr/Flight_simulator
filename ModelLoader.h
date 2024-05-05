#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
};

class ModelLoader {
public:
    Mesh loadModel(const std::string &path);

private:
    void processNode(aiNode *node, const aiScene *scene, Mesh &mesh);
    void processMesh(aiMesh *mesh, const aiScene *scene, Mesh &meshData);
};

#endif
