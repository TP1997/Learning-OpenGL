#include <iostream>
#include <map>
#include "shader.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Image.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#include "vertices.h"
#include "camera.h"

#define PI 3.14159265359

using namespace std;
using namespace sf;

//Globals
const float width=800;
const float height=800;

float lastX=width/2;
float lastY=height/2;
bool firstMouse=true;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

float deltaTime=0.0f;
float lastFrame=0.0f;

uint8_t bitmask=0;

//Func declarations
sf::ContextSettings setSettings();
void checkOglVersion(sf::Window &window);
bool processInput(Window &win);
void moveCamera();
unsigned int loadTexture(const char *path, GLenum);
unsigned int loadCubemap(const char **faces);
Image loadImage(const char *, bool);

int main()
{
    sf::Window window(sf::VideoMode(width, height), "Tut_1", sf::Style::Default, setSettings());
    checkOglVersion(window);

    glewInit();
//Build shaders
    Shader shaderRed("shaders/shader.vs", "shaders/shader.fs");
    Shader shaderGreen("shaders/shader.vs", "shaders/shaderGreen.fs");
    Shader shaderBlue("shaders/shader.vs", "shaders/shaderBlue.fs");
    Shader shaderYellow("shaders/shader.vs", "shaders/shaderYellow.fs");

    glm::vec3 cubePositions[]{
        glm::vec3( -0.75f,  0.75f,  0.0f),
        glm::vec3( 0.75f,  0.75f,  0.0f),
        glm::vec3(-0.75f,  -0.75f,  0.0f),
        glm::vec3(0.75f,  -0.75f,  0.0f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

//Texture loading

//Cube vertex buffer/array
    unsigned int cubeVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);

//Configure uniform buffer object (check shader.vs)
 /*   unsigned int uniformBlockIdxRed    =glGetUniformBlockIndex(shaderRed.ID, "Matrices");
    unsigned int uniformBlockIdxGreen  =glGetUniformBlockIndex(shaderGreen.ID, "Matrices");
    unsigned int uniformBlockIdxBlue   =glGetUniformBlockIndex(shaderBlue.ID, "Matrices");
    unsigned int uniformBlockIdxYellow =glGetUniformBlockIndex(shaderYellow.ID, "Matrices");
    //Set binding points
    glUniformBlockBinding(shaderRed.ID, uniformBlockIdxRed, 0);
    glUniformBlockBinding(shaderGreen.ID, uniformBlockIdxGreen, 0);
    glUniformBlockBinding(shaderBlue.ID, uniformBlockIdxBlue, 0);
    glUniformBlockBinding(shaderYellow.ID, uniformBlockIdxYellow, 0);*/
    //Create uniform buffer object
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2*sizeof(glm::mat4));
//Fill uniform buffer
    //Store projection matrix
    glm::mat4 projection=glm::perspective(glm::radians(45.0f), width/height, 0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //Śtore viewmatrix --> loop

//Depth testing properties
    glEnable(GL_DEPTH_TEST);
//Face culling properties
    glEnable(GL_CULL_FACE);
    glEnable(GL_BACK);
    glFrontFace(GL_CCW);

//Last settings
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    window.setFramerateLimit(60);
    deltaTime=0.0f;
    bool run=true;
    sf::Clock c;
    sf::Mouse::setPosition(Vector2i(1920/2, 1080/2));
    window.setMouseCursorGrabbed(true);
    while(run){
        float currentFrame=c.getElapsedTime().asSeconds();
        deltaTime=currentFrame-lastFrame;
        lastFrame=currentFrame;

        run=processInput(window);
        moveCamera();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//Get and store view matrix to uniform buffer
        glm::mat4 view=camera.GetViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

//DRAW SCENE ...
//Cubes
    //Enable depth buffer writing
    glDepthMask(GL_TRUE);

        glBindVertexArray(cubeVAO);
    //Draw red cubes
        shaderRed.use();
        glm::mat4 model=glm::mat4(1.0f);
        model=glm::translate(model, cubePositions[0]);
        shaderRed.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    //Draw green cubes
        shaderGreen.use();
        model=glm::mat4(1.0f);
        model=glm::translate(model, cubePositions[1]);
        shaderGreen.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    //Draw blue cubes
        shaderBlue.use();
        model=glm::mat4(1.0f);
        model=glm::translate(model, cubePositions[2]);
        shaderGreen.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    //Draw yellow cubes
        shaderYellow.use();
        model=glm::mat4(1.0f);
        model=glm::translate(model, cubePositions[3]);
        shaderGreen.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
//...

        window.display();

    }
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    shaderRed.deleteProgram();
    shaderGreen.deleteProgram();
    shaderBlue.deleteProgram();
    shaderYellow.deleteProgram();

    return 0;
}

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
        else if(event.type==sf::Event::KeyPressed){
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                bitmask|=wmask;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                bitmask|=smask;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                bitmask|=amask;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                bitmask|=dmask;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                return false;
        }
        else if(event.type==sf::Event::KeyReleased){
            if(!sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                bitmask&=~(wmask);
            if(!sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                bitmask&=~(smask);
            if(!sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                bitmask&=~(amask);
            if(!sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                bitmask&=~(dmask);
        }
        else if(event.type==sf::Event::MouseMoved){
            float xpos=sf::Mouse::getPosition().x;
            float ypos=sf::Mouse::getPosition().y;
            if(firstMouse){
                lastX=xpos;
                lastY=ypos;
                firstMouse=false;
            }
            float xoffset=xpos-lastX;
            float yoffset=ypos- lastY;

            lastX=xpos;
            lastY=ypos;

            camera.ProcessMouseMovement(xoffset, yoffset, true);
            //sf::Mouse::setPosition(Vector2i(1920/2, 1080/2));
        }

    }
    return true;

}

void moveCamera(){
    camera.ProcessKeyboard(bitmask, deltaTime);
}

unsigned int loadTexture(const char *path, GLenum interpolation){
    unsigned int texture;
    Image img=loadImage(path, true);

    Vector2u measures=img.getSize();

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, interpolation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, interpolation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    char *data=(char*)img.getPixelsPtr();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, measures.x, measures.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}
unsigned int loadCubemap(const char **faces){
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    for(int n=0; n<6; n++){
        Image img=loadImage(faces[n], false);
        Vector2u measures=img.getSize();
        char *data=(char*)img.getPixelsPtr();
        if(data)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+n, 0, GL_RGBA, measures.x, measures.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else
            cout << "Cubemap texture failed to load at path: " << faces[n] << endl;
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}
Image loadImage(const char *path, bool flip){
    Image img;
    if(!img.loadFromFile(path)){
        cout << "Coud not load " << path << endl;
        return img;
    }
    if(flip) img.flipVertically();
    return img;
}
