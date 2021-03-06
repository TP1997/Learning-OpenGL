#include <iostream>
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
unsigned int loadTexture(char const *path);

int main()
{
    sf::Window window(sf::VideoMode(width, height), "Exc_4", sf::Style::Default, setSettings());
    checkOglVersion(window);
    glewInit();

    Shader cubeShader("shaders/cubeShader.vs", "shaders/cubeShader.fs");
    Shader lampShader("shaders/lampShader.vs", "shaders/lampShader.fs");

    glm::vec3 cubePositions[]{
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 0.0f,  0.0f, 5.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    glm::vec3 lampPositions[]{
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(-0.3f, 0.3f, 1.5f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    glm::vec3 lampColors[]{
        glm::vec3(0.3f, 0.7f, 0.1f),
        glm::vec3(0.8f, 0.4f, 0.2f),
        glm::vec3(0.4f, 0.1f, 0.9f),
        glm::vec3(0.8f, 0.8f, 0.8f)
    };


    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

//Texture loading, diffuse map
    unsigned int texture0=loadTexture("images/container2.png");

//Texture loading, specular map
    unsigned int texture1=loadTexture("images/container2_specular.png");

//Lamp
    unsigned int lampVAO;
    glGenVertexArrays(1, &lampVAO);
    glBindVertexArray(lampVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);

//Fragment shader uniforms
    cubeShader.use();
    //Material attributes
    cubeShader.setInt("material.diffuse", 0);
    cubeShader.setInt("material.specular", 1);
    cubeShader.setFloat("material.shininess", 32.0f);

    //Directional light attributes
    cubeShader.setVec3("dirLight.direction", -0.0f, 0.0f, -5.0f);
    cubeShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    cubeShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    cubeShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    //Point light attributes
    for(int i=0; i<4; i++){
        char buffer[64];

        sprintf(buffer, "pointLight[%i].position", i);
        cubeShader.setVec3(buffer, lampPositions[i]);

        sprintf(buffer, "pointLight[%i].ambient", i);
        cubeShader.setVec3(buffer, 0.05f, 0.05f, 0.05f);
        //cubeShader.setVec3(buffer, lampColors[i]);

        sprintf(buffer, "pointLight[%i].diffuse", i);
        //cubeShader.setVec3(buffer, 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3(buffer, lampColors[i]);

        sprintf(buffer, "pointLight[%i].specular", i);
        //cubeShader.setVec3(buffer, 1.0f, 1.0f, 1.0f);
        cubeShader.setVec3(buffer, lampColors[i]);

        sprintf(buffer, "pointLight[%i].constant", i);
        cubeShader.setFloat(buffer, 1.0f);

        sprintf(buffer, "pointLight[%i].linear", i);
        cubeShader.setFloat(buffer, 0.09f);

        sprintf(buffer, "pointLight[%i].quadratic", i);
        cubeShader.setFloat(buffer, 0.032f);
    }
    //Spotlight attributes
    cubeShader.setFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
    cubeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
    cubeShader.setVec3("spotLight.ambient",0.05f, 0.05f, 0.05f);
    cubeShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
    cubeShader.setVec3("spotLight.specular",1.0f, 1.0f, 1.0f);
    cubeShader.setFloat("spotLight.constant", 1.0f);
    cubeShader.setFloat("spotLight.linear", 0.09f);
    cubeShader.setFloat("spotLight.quadratic", 0.031f);

//Projection matrix
    cubeShader.use();
    glm::mat4 projection=glm::mat4(1.0);
    projection=glm::perspective(glm::radians(45.0f), width/height, 0.1f, 100.0f);
    cubeShader.setMat4("projection", projection);

    lampShader.use();
    lampShader.setMat4("projection", projection);

//Last settings
    window.setFramerateLimit(60);
    glEnable(GL_DEPTH_TEST);
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

        glm::mat4 view=camera.GetViewMatrix();

//Cubes
        cubeShader.use();
    //Camera position for lightning calculations
        cubeShader.setVec3("viewPos", camera.Position);
    //Cube view transformation
        cubeShader.setMat4("view", view);
    // Set spotlight position & direction
        cubeShader.setVec3("spotLight.position", camera.Position);
        cubeShader.setVec3("spotLight.spotDir", camera.Front);
    //Bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture0);
    //Bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glBindVertexArray(cubeVAO);

        for(unsigned int n=0; n<10; n++){
        //Cube world transformation
            glm::mat4 model=glm::mat4(1.0f);
            model=glm::translate(model, cubePositions[n]);
            float angle=20.0f*n*currentFrame;
            model=glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            cubeShader.setMat4("model", model);
        //Render cubes
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

//Point lights
        lampShader.use();
    //Lamp view transformation
        lampShader.setMat4("view", view);

        glBindVertexArray(lampVAO);
        for(unsigned int n=0; n<4; n++){
            //Lamp world transformation
            glm::mat4 model=glm::mat4(1.0f);
            model=glm::translate(model, lampPositions[n]);
            model=glm::scale(model, glm::vec3(0.2f));
            lampShader.setMat4("model", model);
            lampShader.setVec3("fragCol", lampColors[n]);
            //Render lamp

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        window.display();

    }
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lampVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture0);
    glDeleteTextures(1, &texture1);
    cubeShader.deleteProgram();
    lampShader.deleteProgram();

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
    camera.ProcessKeyboard(bitmask, true);
}

unsigned int loadTexture(char const *path){
    unsigned int texture;
    Image img;
    if(!img.loadFromFile(path)){
        cout << "Could not load " << path << endl;
        return texture;
    }
    img.flipVertically();
    Vector2u measures=img.getSize();

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    char *data=(char*)img.getPixelsPtr();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, measures.x, measures.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}
