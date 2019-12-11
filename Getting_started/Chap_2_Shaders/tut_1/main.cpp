#include <iostream>
#include <math.h>

#include <GL/glew.h>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

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

const char *vertexShaderSrc="#version 330 core\n"
                             "layout (location=0) in vec3 aPos;\n"
                             "void main(){\n"
                             "gl_Position=vec4(aPos, 1.0); }";

const char *fragmentShaderSrc="#version 330 core\n"
                              "out vec4 FragColor;\n"
                              "uniform vec4 ourColor;"
                              "void main(){\n"
                              "FragColor=ourColor; }";

int main()
{
    const int width=800; const int height=800;
    sf::Window window(sf::VideoMode(width, height), "OpenGL_tut1 works!", sf::Style::Default, setSettings());
    checkOglVersion(window);
    glewInit();

    //Create vertex shader
    int vertexShader=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //Create fragment shader
    int fragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //Linking shaders
    int shaderProgram=glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    //Deleting shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //Vertex data setup
    float vertices[]={-0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.0f,  0.5f, 0.0f };

    //VertexBufferObject, VertexArrayObject
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Linking vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    bool run=true;
    sf::Clock c;
    while(run){
        float timeVal=c.getElapsedTime().asSeconds();
        float greenVal=sin(timeVal)/2.0f + 0.5f;
        float blueVal=sin(timeVal*PI)/2.0f + 0.5f;
        float redVal=sin(timeVal*PI/2)/2.0f + 0.5f;
        int vertexColorLoc=glGetUniformLocation(shaderProgram, "ourColor");
        run=processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //Draw triangle
        glUseProgram(shaderProgram);
        glUniform4f(vertexColorLoc, redVal, greenVal, blueVal, 1.0f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        window.display();
        cout << greenVal << endl;
    }

    //De-allocating rescoures (optional)
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    cout << "moi" << endl;

    return 0;
}
