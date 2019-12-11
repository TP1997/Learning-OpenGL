#include <iostream>

#include "shader.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Image.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define PI 3.14159265359

using namespace std;
using namespace sf;

sf::ContextSettings setSettings(){

    sf::ContextSettings settings;
    settings.depthBits=24;
    settings.stencilBits=8;
    settings.antialiasingLevel=4;
    settings.majorVersion=4;
    settings.minorVersion=6;

    return settings;

}

void checkOglVersion(sf::Window &window){

    sf::ContextSettings settings=window.getSettings();
    cout << "OpenGL version: " << settings.majorVersion << "." << settings.minorVersion << endl;
}

bool processInput(Window &win){
    Event event;
    while(win.pollEvent(event)){
        if(event.type==sf::Event::Closed)
            return false;
    }
    return true;

}
Image loadImage(string path){
    Image img_data;
    if(!img_data.loadFromFile(path)){
        cout << "Could not load " << path << endl;
    }
    img_data.flipVertically();
    return img_data;

}


int main()
{
    const int width=800; const int height=800;
    sf::Window window(sf::VideoMode(width, height), "OpenGL_tut1 works!", sf::Style::Default, setSettings());
    checkOglVersion(window);
    glewInit();

    Shader ourShader("shaders/shader.vs", "shaders/shader.fs");
    Image img1=loadImage("images/container.jpg");
    Image img2=loadImage("images/awesomeface.png");
    Vector2u measures1=img1.getSize();
    Vector2u measures2=img2.getSize();

    //Vertex data setup
    float vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
};
    unsigned int indicies[]={0,1,3,
                             1,2,3};

    unsigned int VBO, VAO, EBO, texture1, texture2;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
//Texture 1
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    char *data1=(char*)img1.getPixelsPtr();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, measures1.x, measures1.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
    glGenerateMipmap(GL_TEXTURE_2D);
//Texture 2
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    char *data2=(char*)img2.getPixelsPtr();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, measures2.x, measures2.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    glGenerateMipmap(GL_TEXTURE_2D);


    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.use();
    ourShader.setInt("texture2", 1);

//Transformations
/*
    glm::mat4 trans=glm::mat4(1.0f);
    trans=glm::translate(trans, glm::vec3(0.5, -0.0, 0.0));
    trans=glm::rotate(trans, (float)(PI/3), glm::vec3(0.0, 0.0, 1.0));
    trans=glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));


    unsigned int trasformLoc=glGetUniformLocation(ourShader.ID, "transform");
    glUniformMatrix4fv(trasformLoc, 1, GL_FALSE, glm::value_ptr(trans));*/

    bool run=true;
    sf::Clock c;
    while(run){
        run=processInput(window);
        float time=c.getElapsedTime().asSeconds();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
//transformations 1
        glm::mat4 trans1=glm::mat4(1.0f);
        trans1=glm::translate(trans1, glm::vec3(0.5, 0.0, 0.0));
        trans1=glm::rotate(trans1, time, glm::vec3(0.0, 0.0, 1.0));
        trans1=glm::scale(trans1, glm::vec3(0.5, 0.5, 0.5));
        ourShader.setMat4("transform", trans1);

        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

//transformations 2
        glm::mat4 trans2=glm::mat4(1.0);
        trans2=glm::translate(trans2, glm::vec3(-0.5, 0.0, 0.0));
        trans2=glm::rotate(trans2, time, glm::vec3(0.0, 0.0, 1.0));
        trans1=glm::scale(trans2, glm::vec3(sin(time))*glm::vec3(0.5, 0.5, 0.5));
        ourShader.setMat4("transform", trans1);

        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        window.display();

    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    ourShader.deleteProgram();

    return 0;
}
