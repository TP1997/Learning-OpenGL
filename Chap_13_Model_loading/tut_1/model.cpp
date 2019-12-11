#include <cstring>

#include "model.h"

Model::Model(const char *path, bool gamma): gammaCorrection(gamma){
    loadModel(path);
}

void Model::draw(Shader shader){
    //draw all meshe's of the scene
    for(Mesh mesh : meshes){
        mesh.draw(shader);
    }
}

void Model::loadModel(string path){
    Assimp::Importer importer;
    //load 3D-model to aiScene-object
    const aiScene *scene=importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
        return;
    }
    directory=path.substr(0, path.find_last_of('/'));
    //process node's of the scene
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene){
    //process all current node's meshes
    for(unsigned int n=0; n<node->mNumMeshes; n++){
        aiMesh *mesh=scene->mMeshes[node->mMeshes[n]];
        meshes.push_back(processMesh(mesh, scene));
    }
    //continue to current node's children
    for(unsigned int n=0; n<node->mNumChildren; n++){
        processNode(node->mChildren[n], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene){
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    //process vertices
    for(unsigned int n=0; n<mesh->mNumVertices; n++){
        Vertex vertex;
        glm::vec3 vec;
        //vertec positions
        vec.x=mesh->mVertices[n].x;
        vec.y=mesh->mVertices[n].y;
        vec.z=mesh->mVertices[n].z;
        vertex.Position=vec;
        //vertex normals
        vec.x=mesh->mNormals[n].x;
        vec.y=mesh->mNormals[n].y;
        vec.z=mesh->mNormals[n].z;
        vertex.Normal=vec;
        //vertex texture positions
        if(mesh->mTextureCoords[0]){
            glm::vec2 tvec;
            tvec.x=mesh->mTextureCoords[0][n].x;
            tvec.y=mesh->mTextureCoords[0][n].y;
            vertex.TexCoords=vec;
        }
        else
            vertex.TexCoords=glm::vec2(0.0f, 0.0f);

        if(mesh->HasTangentsAndBitangents()){
            //vertex tangents
            vec.x=mesh->mTangents[n].x;
            vec.y=mesh->mTangents[n].y;
            vec.z=mesh->mTangents[n].z;
            vertex.Tangent=vec;
            //vertex bitangents
            vec.x=mesh->mBitangents[n].x;
            vec.y=mesh->mBitangents[n].y;
            vec.z=mesh->mBitangents[n].z;
            vertex.Bitangent=vec;
        }
        else{
            vertex.Tangent=glm::vec3(0.0f);
            vertex.Bitangent=glm::vec3(0.0f);
        }
    }
    //process indicies
    for(uint8_t n=0; n<mesh->mNumFaces; n++){
        aiFace face=mesh->mFaces[n];
        for(uint8_t m=0; m<face.mNumIndices; m++){
            indices.push_back(face.mIndices[m]);
        }
    }
    //process material
    if(mesh->mMaterialIndex>=0){
        aiMaterial *material=scene->mMaterials[mesh->mMaterialIndex];
        //load diffuse maps
        vector<Texture> diffuseMaps=loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        //load specular maps
        vector<Texture> specularMaps=loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }
    return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName){
    vector<Texture> textures;
    for(uint8_t n=0; n<mat->GetTextureCount(type); n++){
        //get texture data
        aiString str;
        mat->GetTexture(type, n, &str);
        bool skip=false;
        for(uint8_t m=0; m<textures_loaded.size(); m++){
            if(std::strcmp(textures_loaded[m].path.data(), str.C_Str())==0){
                textures.push_back(textures_loaded[m]);
                skip=true;
            }
        }
        if(!skip){
            //build texture to Texture-struct-format
            Texture texture;
            texture.id=TextureFromFile(str.C_Str(), directory);
            texture.type=typeName;
            texture.path=string(str.C_Str());
            textures.push_back(texture);
        }
    }
    return textures;
}

//helper function
unsigned int TextureFromFile(const char *path, const string directory, bool gamma){
    unsigned int texture;
    sf::Image img;
    string path_=directory + '/' + string(path);

    if(!img.loadFromFile(path_)){
        cout << "Could not load " << path_ << endl;
        return texture;
    }

    glGenTextures(1, &texture);

    sf::Vector2u measures=img.getSize();
    char *data=(char*)img.getPixelsPtr();
    if(data){
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, measures.x, measures.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return texture;
}
