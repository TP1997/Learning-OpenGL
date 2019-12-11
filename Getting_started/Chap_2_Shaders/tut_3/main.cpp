#include <iostream>
#include <GL/glew.h>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include "shader.h"


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

int main()
{
    const int width=800; const int height=800;
    sf::Window window(sf::VideoMode(width, height), "OpenGL_tut1 works!", sf::Style::Default, setSettings());
    checkOglVersion(window);
    glewInit();

    //Create vertex shader
    Shader ourShader("shaders/shader.vs", "shaders/shader.fs");

    //Vertex data setup
    float vertices[]={-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
                       0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f};

    //VertexBufferObject, VertexArrayObject
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Linking vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    float offset=0.5f;
    bool run=true;
    while(run){
        run=processInput(window);

        ourShader.setFloat("xOffset", offset);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //Draw triangle
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        window.display();
        offset=(offset>1.0)? -1.0f : offset+0.0001f;

    }

    //De-allocating rescoures (optional)
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    cout << "moi" << endl;

    return 0;
}
