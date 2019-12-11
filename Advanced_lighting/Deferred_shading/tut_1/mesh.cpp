#include "mesh.h"

/** Constructor
* @param vertices Mesh's vertex data.
* @param indices Mesh's index data.
* @param textures Mesh's texture data.
*/
Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures){
    this->vertices=vertices;
    this->indices=indices;
    this->textures=textures;

    setupMesh();
}

/** Destuctor
* Deletes all mesh's rescoures properly.
*/
Mesh::~Mesh(){
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

/** Initalize all buffer objects & arrays.
*
*/
void Mesh::setupMesh(){
    //Create buffers & arrays.
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    //Fill buffer (VBO) with vertex data.
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    //Link vertex attributes (spacing between attributes).
    //1. Vertex position.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    //2. Vertex normal vectors.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    //3. Vertex texture coordinates.
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);
    //4. Vertex tangent vecor.
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(3);
    //4. Vertex bitangent vector.
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    glEnableVertexAttribArray(4);

    //Fill buffer (EBO) with index data.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}

/** Draw mesh object.
* @param shader Used shader-program.
*/
void Mesh::draw(Shader &shader){
    //Variables to hold appropriate textures.
    unsigned int diffuseNr=1;
    unsigned int specularNr=1;
    unsigned int normalNr=1;
    unsigned int heightNr=1;
    //Loop over textures defined to current mesh. Activate & bind all textures used.
    for(unsigned int n=0; n<textures.size(); n++){
        //Activate correct texture.
        glActiveTexture(GL_TEXTURE0+n);
        //Retrieve texture type & number.
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

        shader.setInt((name+number).c_str(), n);
        //std::cout << (name+number).c_str() << " : " << n << std::endl;
        glBindTexture(GL_TEXTURE_2D, textures[n].id);
    }
    //Draw mesh.
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    //std::cout << indices.size() << std::endl;
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //Reseting to defaults.
    glActiveTexture(GL_TEXTURE0);

}
