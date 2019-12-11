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
    Shader shader("shaders/shader.vs", "shaders/shader.fs");
    Shader lampShader("shaders/lampShader.vs", "shaders/lampShader.fs");
//Texture loading
    unsigned int texture0=loadTexture("images/wood.jpg", GL_REPEAT);
//Lamp attributes
    glm::vec3 lampPosition=glm::vec3( 0.0f,  1.0f,  0.0f);
    glm::vec3 lampColor=glm::vec3( 0.8f, 0.8f, 0.8f);

//VERTEX ARRAYS...
//Plane vertex buffer/array
    unsigned int planeVBO, planeVAO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int normVBO;
    glGenBuffers(1, &normVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, normVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), &normals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(2);

//Lamp vertex/buffer array
    unsigned int lampVBO, lampVAO;
    glGenVertexArrays(1, &lampVAO);
    glGenBuffers(1, &lampVBO);

    glBindVertexArray(lampVAO);
    glBindVertexArray(lampVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), &lampVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
//...

//Set uniforms...
//Plane shader
    shader.use();
//Plane shader.vs uniforms
    //Model matrix
    glm::mat4 model=glm::mat4(1.0f);
    shader.setMat4("model", model);
    //Projection matrix
    glm::mat4 projection=glm::mat4(1.0);
    projection=glm::perspective(glm::radians(45.0f), width/height, 0.1f, 100.0f);
    shader.setMat4("projection", projection);
//Plane shader.fs uniforms
    //Texture
    shader.setInt("texture_", 0);
    //Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    //PointLight structure
    shader.setVec3("light.position", lampPosition);
    shader.setVec3("light.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    shader.setVec3("light.diffuse", lampColor);
    shader.setVec3("light.specular", lampColor);
    //Other uniforms
    shader.setBool("blinn", false);

//Lamp shader
    lampShader.use();
//Lamp shader.vs uniforms
    //Model matrix
    model=glm::translate(model, lampPosition);
    model=glm::scale(model, glm::vec3(0.2f));
    lampShader.setMat4("model", model);
    //Projection
    lampShader.setMat4("projection", projection);
    //Lamp color
    lampShader.setVec3("lightColor", lampColor);
//Lamp shader.fs uniforms

//...

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
    //measure time
        float currentFrame=c.getElapsedTime().asSeconds();
        deltaTime=currentFrame-lastFrame;
        lastFrame=currentFrame;
    //process input
        run=processInput(window);
        moveCamera();
    //clear framebuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//Get and set view matrix & view position
        glm::mat4 view=camera.GetViewMatrix();
        shader.use();
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);
        lampShader.use();
        lampShader.setMat4("view", view);
//DRAW SCENE ...
//Plane
        shader.use();
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
//Lamp
        lampShader.use();
        glBindVertexArray(lampVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
//...

        window.display();

    }
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteTextures(1, &texture0);

    shader.deleteProgram();

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
