#include <cstring>

#include "model.h"
/** Constructor
* @param path Path to the directory holding the model files.
* @param gamma Is gamma correction used.
*/
Model::Model(const char *path, bool gamma): gammaCorrection(gamma){
    loadModel(path);
}

/** Draws the model and thus all it's meshes.
* @param shader Used shader program.
*/
void Model::draw(Shader &shader){
    for(unsigned int n=0; n<meshes.size(); n++){
        //meshes[n].draw(shader);
    }
    for(Mesh &mesh : meshes){
        mesh.draw(shader);
    }
}

/** Loads model with supported ASSIMP-extensions. Stores resulting meshes in the meshes-vector.
* @param path Path to the model directory.
*/
void Model::loadModel(string const &path){
    Assimp::Importer importer;
    //load 3D-model to aiScene-object
    const aiScene *scene=importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    //Check for nullptr, errorflags, empty scene.
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
        return;
    }
    //Separate directory from file path.
    directory=path.substr(0, path.find_last_of('/'));
    //process ASSIMP-node's recursively.
    processNode(scene->mRootNode, scene);
}

/** Processes ASSIMP-node(s) recursively. Process each individual mesh located at the node.
* @param node Current root node.
* @param scene Scene-object, which holds all the data of the model.
*/
void Model::processNode(aiNode *node, const aiScene *scene){
    //Process all current node's meshes.
    for(unsigned int n=0; n<node->mNumMeshes; n++){
        aiMesh *mesh=scene->mMeshes[node->mMeshes[n]];
        meshes.push_back(processMesh(mesh, scene));
    }
    //Continue to current node's children.
    for(unsigned int n=0; n<node->mNumChildren; n++){
        processNode(node->mChildren[n], scene);
    }
}

/** Process ASSIMP mesh & creates Mesh object based on it.
* @param mesh Mesh object.
* @param scene Scene object which holds all the data of the model.
* @return Created mesh object.
*/
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene){
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    //Loop over all verticies the mesh contains & collect neccessery data.
    for(unsigned int n=0; n<mesh->mNumVertices; n++){
        Vertex vertex;
        glm::vec3 vec;                  //Placeholder vector since assimp uses it's own vector class which doesn't directly convert to glm vector classes.
        //1. Vertex positions.
        vec.x=mesh->mVertices[n].x;
        vec.y=mesh->mVertices[n].y;
        vec.z=mesh->mVertices[n].z;
        vertex.Position=vec;

        //2. Vertex normals.
        vec.x=mesh->mNormals[n].x;
        vec.y=mesh->mNormals[n].y;
        vec.z=mesh->mNormals[n].z;
        vertex.Normal=vec;

        //3. Vertex texture positions (if them exist).
        if(mesh->mTextureCoords[0]){
            glm::vec2 tvec;
            //Single vertex can contain up to 8 different texture coordinates. We use only the first set.
            tvec.x=mesh->mTextureCoords[0][n].x;
            tvec.y=mesh->mTextureCoords[0][n].y;
            vertex.TexCoords=vec;
        }
        else
            vertex.TexCoords=glm::vec2(0.0f, 0.0f);

        //4 & 5. Tangents & Bitangents (if them exist).
        if(mesh->HasTangentsAndBitangents()){
            vec.x=mesh->mTangents[n].x;
            vec.y=mesh->mTangents[n].y;
            vec.z=mesh->mTangents[n].z;
            vertex.Tangent=vec;

            vec.x=mesh->mBitangents[n].x;
            vec.y=mesh->mBitangents[n].y;
            vec.z=mesh->mBitangents[n].z;
            vertex.Bitangent=vec;
        }
        else{
            vertex.Tangent=glm::vec3(0.0f);
            vertex.Bitangent=glm::vec3(0.0f);
        }

        //All vertex data collected.
        vertices.push_back(vertex);
    }

    //Loop over all faces (triangles) mesh contains. Retrieve corresponding vertex indicies of each face of the mesh.
    for(unsigned int n=0; n<mesh->mNumFaces; n++){
        aiFace face=mesh->mFaces[n];
        for(uint8_t m=0; m<face.mNumIndices; m++){
            indices.push_back(face.mIndices[m]);
        }
    }

    //Process materials of the mesh (if them exist).
    if(mesh->mMaterialIndex>=0){
        aiMaterial *material=scene->mMaterials[mesh->mMaterialIndex];

        //1. Diffuse maps.
        vector<Texture> diffuseMaps=loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        //2. Specular maps.
        vector<Texture> specularMaps=loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        //3. Normal maps.
        vector<Texture> normalMaps=loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        //4. Height (depth) maps.
        vector<Texture> heightMaps=loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    }
    //All mesh data collected.
    return Mesh(vertices, indices, textures);
}

/** Checks all material textures of given type & loads the textures if they are not already loaded.
* @param mat Mesh's material object.
* @param type Material texture's type specifiation.
* @param typeName Defined typename
* @return Collection of material textures of given type.
*/
vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName){
    vector<Texture> textures;
    //Loop over all material textures of given type.
    for(unsigned int n=0; n<mat->GetTextureCount(type); n++){
        //Get texture data of given type.
        aiString str;
        mat->GetTexture(type, n, &str);
        //Loop over loaded textures & check if current texture is already loaded.
        bool skip=false;
        for(unsigned int m=0; m<textures_loaded.size(); m++){
            //If current texture is already loaded, add it to the collection.
            if(std::strcmp(textures_loaded[m].path.data(), str.C_Str())==0){
                textures.push_back(textures_loaded[m]);
                skip=true;
            }
        }
        //If texture wasn't already loaded.
        if(!skip){
            //Construct Texture-object based on loaded (new) texture.
            Texture texture;
            texture.id=TextureFromFile(str.C_Str(), directory);
            texture.type=typeName;
            texture.path=string(str.C_Str());
            textures.push_back(texture);
        }
    }
    return textures;
}

/** Helper function to load texture from file.
* @param path Path to the texture.
* @param directory Texture's directory.
* @param gamma ???
* @return Id (OpenGL) of the loaded texture
*/
unsigned int TextureFromFile(const char *path, const string directory, bool gamma){
    string path_=directory + '/' + string(path);
    unsigned int textureID;
    glGenTextures(1, &textureID);

    unsigned int width, height;
    unsigned char *data=loadImageData(path_.c_str(), width, height);
    if(data){
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return textureID;
}
/** Helper function to load image data.
* @param path Path to the image data.
* @param width Reference to width-variable.
* @param height Reference to height-variable.
* @param flip Does image need flipping?
* @return Pointer to image's pixel data.
*/
unsigned char* loadImageData(const char *path, unsigned int &width, unsigned int &height, bool flip){
    sf::Image img;
    if(!img.loadFromFile(path)){
        cout << "Coud not load image: " << path << endl;
        return nullptr;
    }
    if(flip)
        img.flipVertically();

    sf::Vector2u measures=img.getSize();
    width=measures.x;
    height=measures.y;
    unsigned char *data=(unsigned char*)img.getPixelsPtr();

    return data;

}
