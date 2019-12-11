#include <iostream>
#include "shader.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Image.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#include "vertices.h"

#define PI 3.14159265359

using namespace std;
using namespace sf;

glm::vec3 cameraPos;
glm::vec3 cameraUp;
glm::vec3 cameraFront;
float deltaTime;
float lastX=400.0f;
float lastY=400.0f;
bool firstMouse=true;
float pitch=0.0f;
float yaw=-89.3f;

uint8_t wmask=1<<0;
uint8_t smask=1<<1;
uint8_t amask=1<<2;
uint8_t dmask=1<<3;
uint8_t bitmask=0;

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
            float xPos=sf::Mouse::getPosition().x;
            float yPos=sf::Mouse::getPosition().y;
            if(firstMouse){
                lastX=xPos;
                lastY=yPos;
                firstMouse=false;
            }
            float xoffset=(xPos-lastX)*0.005;
            float yoffset=(yPos-lastY)*0.005;
            lastX=xPos;
            lastY=yPos;

            pitch+=yoffset;
            yaw+=xoffset;
            if(pitch>89.0f){
                pitch=89.0f;
            }
            if(pitch<-89.0f){
                pitch=-89.0f;
            }
            glm::vec3 front_;
            front_.x=cos(pitch)*cos(yaw);
            front_.y=sin(pitch);
            front_.z=cos(pitch)*sin(yaw);
            cameraFront=glm::normalize(front_);
        }

    }
    return true;

}
void moveCamera(){
    float cameraSpeed=deltaTime*5.5f;
    if(bitmask&wmask)
        cameraPos+=cameraSpeed*cameraFront;
    if(bitmask&smask)
        cameraPos-=cameraSpeed*cameraFront;
    if(bitmask&amask)
        cameraPos-=cameraSpeed*glm::normalize(glm::cross(cameraFront, cameraUp));
    if(bitmask&dmask)
        cameraPos+=cameraSpeed*glm::normalize(glm::cross(cameraFront, cameraUp));
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

    glm::vec3 cubePositions[]{
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };


    unsigned int VBO, VAO, texture1, texture2;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

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
    ourShader.setInt("texture2", 1);
//Camera Position
    cameraPos=glm::vec3(0.0f, 0.0f, 3.0f);
    cameraFront=glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp=glm::vec3(0.0f, 1.0f, 0.0f);

//Projection matrix
    ourShader.use();
    glm::mat4 projection=glm::mat4(1.0);
    projection=glm::perspective(glm::radians(45.0f), width/height, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);

    window.setFramerateLimit(60);
    //window.setMouseCursorVisible(false);
    glEnable(GL_DEPTH_TEST);
    deltaTime=0.0f;
    float lastFrame=0.0f;
    bool run=true;
    sf::Clock c;
    sf::Mouse::setPosition(Vector2i(1920/2-200, 1080/2));
    while(run){
        //lastFrame=c.getElapsedTime().asSeconds();
        float currentFrame=c.getElapsedTime().asSeconds();
        deltaTime=currentFrame-lastFrame;
        lastFrame=currentFrame;

        run=processInput(window);
        moveCamera();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glm::mat4 view=glm::lookAt(cameraPos, cameraPos+cameraFront, cameraUp);
        ourShader.setMat4("view", view);

        glBindVertexArray(VAO);
        for(unsigned int n=0; n<10; n++){

            glm::mat4 model=glm::mat4(1.0f);
            model=glm::translate(model, cubePositions[n]);

            float angle=20.0f*n;
            model=glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            ourShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        window.display();
        cout << deltaTime << endl;

    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    ourShader.deleteProgram();

    return 0;
}
