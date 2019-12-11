#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <SFML/Graphics/Image.hpp>

#include <fstream>

#include "mesh.h"

class Model{
    public:
    Model(const char*, bool gamma=false);
    void draw(Shader);
    private:
    vector<Mesh> meshes;
    string directory;
    vector<Texture> textures_loaded;
    bool gammaCorrection;
    //model building functions (called from constructor)
    void loadModel(string);
    void processNode(aiNode*, const aiScene*);
    Mesh processMesh(aiMesh*, const aiScene*);
    vector<Texture> loadMaterialTextures(aiMaterial*, aiTextureType, string);
};

unsigned int TextureFromFile(const char*, const string, bool gamma=false);
#endif // MODEL_H_INCLUDED
