#include "ModelLoader.h"

Mesh ModelLoader::loadModel(const std::string &path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr<<("ERROR::ASSIMP::" + std::string(importer.GetErrorString()));
    }

    Mesh mesh;
    processNode(scene->mRootNode, scene, mesh);
    return mesh;
}

void ModelLoader::processNode(aiNode *node, const aiScene *scene, Mesh &mesh) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[i]];
        this->processMesh(ai_mesh, scene, mesh); // Przekazuj odwo�anie do mesh
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, mesh);
    }
}

void ModelLoader::processMesh(aiMesh *ai_mesh, const aiScene *scene, Mesh &meshData) {
    meshData.vertices.reserve(ai_mesh->mNumVertices);
    meshData.normals.reserve(ai_mesh->mNumVertices);
    meshData.texCoords.reserve(ai_mesh->mNumVertices);


    for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++) {
        // Przetwarzanie wierzcho�k�w
        glm::vec3 vertex(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z);
        meshData.vertices.push_back(vertex);

        // Przetwarzanie normalnych
        glm::vec3 normal(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z);
        meshData.normals.push_back(normal);

        // Wczytywanie koordynat�w tekstur, je�li s� dost�pne
        if (ai_mesh->HasTextureCoords(0)) {
            glm::vec2 texCoord(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y);
            meshData.texCoords.push_back(texCoord);
        }
    }
}