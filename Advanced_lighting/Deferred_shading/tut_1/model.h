#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <SFML/Graphics/Image.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "mesh.h"
#include "shader.h"

using namespace std;
class Model{
public:
    vector<Texture> textures_loaded;        //Stores all the textures loaded so far (used for optimization purposes)
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;

    Model(const char *path, bool gamma=false);
    void draw(Shader &shader);
private:
    void loadModel(string const &path);
    void processNode(aiNode*, const aiScene*);
    Mesh processMesh(aiMesh*, const aiScene*);
    vector<Texture> loadMaterialTextures(aiMaterial*, aiTextureType, string);
};

unsigned int TextureFromFile(const char*, const string, bool gamma=false);
unsigned char* loadImageData(const char *path, unsigned int &width, unsigned int &height, bool flip=true);
#endif // MODEL_H_INCLUDED
