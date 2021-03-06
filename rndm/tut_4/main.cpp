#include <iostream>

#include <GL/glew.h>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>


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

void checkVtxAttributes(){
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    cout << "Maximum nr of vertex attributes supported: " << nrAttributes << endl;
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
                             "gl_Position=vec4(aPos.x, aPos.y, aPos.z, 1.0); }";

const char *fragmentShader1Src="#version 330 core\n"
                              "out vec4 FragColor;\n"
                              "void main(){\n"
                              "FragColor=vec4(1.0f, 0.5f, 0.2f, 1.0f); }";

const char *fragmentShader2Src="#version 330 core\n"
                              "out vec4 FragColor;\n"
                              "void main(){\n"
                              "FragColor=vec4(1.0f, 1.0f, 0.0f, 1.0f); }";

int main()
{
    const int width=800; const int height=800;
    sf::Window window(sf::VideoMode(width, height), "OpenGL_tut1 works!", sf::Style::Default, setSettings());
    checkOglVersion(window);
    checkVtxAttributes();
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

    //Create fragment shader1
    int fragmentShader=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShader1Src, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //Create fragment shader2
    int fragmentShader2=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader2, 1, &fragmentShader2Src, NULL);
    glCompileShader(fragmentShader2);

    glGetShaderiv(fragmentShader2, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader2, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //Linking shaders 1
    int shaderProgram1=glCreateProgram();
    glAttachShader(shaderProgram1, vertexShader);
    glAttachShader(shaderProgram1, fragmentShader);
    glLinkProgram(shaderProgram1);

    glGetProgramiv(shaderProgram1, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram1, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    //Linking shaders 2
    int shaderProgram2=glCreateProgram();
    glAttachShader(shaderProgram2, vertexShader);
    glAttachShader(shaderProgram2, fragmentShader2);
    glLinkProgram(shaderProgram2);

    glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram2, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    //Deleting shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(fragmentShader2);

    //Vertex data setup
    float vertices1[]={0.0f,  0.0f, 0.0f,
                       0.0f, -0.5f, 0.0f,
                      -0.5f, -0.5f, 0.0f };

    float vertices2[]={0.5f,  0.0f, 0.0f,
                       0.0f, -0.5f, 0.0f,
                      -0.5f, 0.5f, 0.0f };

    //VertexBufferObject, VertexArrayObject
    unsigned int VBO[2], VAO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
////////////////////////////////////////////7
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
/////////////////////////////////////////////////7
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
//////////////////////////////////////////////////7

    //Linking vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bool run=true;
    while(run){
        run=processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        //Draw triangle
        glUseProgram(shaderProgram1);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(shaderProgram2);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        window.display();
    }

    //De-allocating rescoures (optional)
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);

    glDeleteProgram(shaderProgram1);
    glDeleteProgram(shaderProgram2);


    return 0;
}
