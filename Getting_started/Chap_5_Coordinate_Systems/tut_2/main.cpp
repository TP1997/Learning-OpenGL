#include <iostream>
#include "shader.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Image.hpp>
//#include <SFML/Graphics.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vertices.h"

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
    const float width=800; const float height=800;
    sf::Window window(sf::VideoMode(width, height), "OpenGL_tut1 works!", sf::Style::Default, setSettings());
    checkOglVersion(window);
    glewInit();

    Shader ourShader("shaders/shader.vs", "shaders/shader.fs");
    Image img1=loadImage("images/container.jpg");
    Image img2=loadImage("images/awesomeface.png");
    Vector2u measures1=img1.getSize();
    Vector2u measures2=img2.getSize();


    unsigned int VBO, VAO, EBO, texture1, texture2;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

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


    window.setFramerateLimit(60);
    glEnable(GL_DEPTH_TEST);
    bool run=true;
    sf::Clock c;
    while(run){
        run=processInput(window);
        float time=c.getElapsedTime().asSeconds();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
//Transformations
        ourShader.use();
        glm::mat4 model=glm::mat4(1.0);
        model=glm::rotate(model, time*glm::radians(55.0f), glm::vec3(1.0f, 1.0f, 1.0f));


        glm::mat4 view=glm::mat4(1.0);
        view=glm::translate(view, glm::vec3(0.0, 0.0, -3.0));


        glm::mat4 projection=glm::mat4(1.0);
        projection=glm::perspective(glm::radians(45.0f), width/height, 0.1f, 100.0f);
        //projection=glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);

        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
//

        ourShader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        window.display();

    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    ourShader.deleteProgram();

    return 0;
}
