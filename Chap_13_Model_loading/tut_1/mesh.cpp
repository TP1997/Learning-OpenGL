#include "mesh.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures){
    this->vertices=vertices;
    this->indices=indices;
    this->textures=textures;

    setupMesh();
}
Mesh::~Mesh(){
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

//Setup shader data
void Mesh::setupMesh(){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    //vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    //vertex normal vectors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    //vertex texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);
    //vertex tangent vecor
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(3);
    //vertex bitangent vector
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
}

void Mesh::draw(Shader shader){
    unsigned int diffuseNr=0;
    unsigned int specularNr=0;
    unsigned int normalNr=0;
    unsigned int heightNr=0;
    for(unsigned int n=0; n<textures.size(); n++){
        glActiveTexture(GL_TEXTURE0+n);

        string number;
        string name=textures[n].type;
        if(name=="texture_diffuse")
            number=to_string(diffuseNr++);
        else if (name=="texture_specular")
            number=to_string(specularNr++);
        else if(name=="texture_normal")
            number=to_string(normalNr++);
        else if(name=="texture_height")
            number=to_string(heightNr++);

        shader.setFloat(("material."+name+number).c_str(), n);
        glBindTexture(GL_TEXTURE_2D, textures[n].id);
    }
    //draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);

}
