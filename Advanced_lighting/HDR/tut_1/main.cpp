#include "shader.h"
#include "vertices.h"
#include "camera.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Image.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#define PI 3.14159265359

using namespace std;
using namespace sf;

//Globals
const float SCR_W=800;
const float SCR_H=800;
const unsigned int SHDW_W=1024;
const unsigned int SHDW_H=1024;

float lastX=SCR_W/2;
float lastY=SCR_H/2;
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
Image loadImage(const char *, bool);
unsigned int createPlaneVAO();

void renderScene(const Shader &shader, const unsigned int &cubeVAO);
void renderCube(const unsigned int &cubeVAO);
void renderQuad(const unsigned int &quadVAO);
/***********************************************************************************************/
/***************************** MAIN FUNCTION START *********************************************/
int main(){
    sf::Window window(sf::VideoMode(SCR_W, SCR_H), "Parallax mapping 1.", sf::Style::Default, setSettings());
    checkOglVersion(window);
    glewInit();

/**Shader Build...                          */
    Shader shader("shaders/shader.vs", "shaders/shader.fs");
    Shader hdrShader("shaders/hdrShader.vs", "shaders/hdrShader.fs");
/**...                                      */
/**Texture loading...                       */
    unsigned int woodTex=loadTexture("images/wood.png", GL_REPEAT);
/**...                                      */
/**Light properties...                      */
    float exposure=0.1f;
    int lightCount=4;
    glm::vec3 lightPos[]={ glm::vec3( 0.0f,  0.0f, 49.5f),      //Back light
                           glm::vec3(-1.4f, -1.9f, 19.0f),
                           glm::vec3( 0.0f, -1.8f, 14.0f),
                           glm::vec3( 0.8f, -1.7f, 16.0f) };

    glm::vec3 lightCol[]={ glm::vec3(200.f, 200.f, 100.f),      //Back light
                           glm::vec3(0.1f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, 0.2f),
                           glm::vec3(0.0f, 0.1f, 0.0f) };
/**...                                      */
/**VBO/VAO/FBO/TBO...                       */
//Cube
    unsigned int cubeVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    //Fill buffer with vertex data.
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    //Link vertex attributes (spacing between attributes).
    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
//Quad
    unsigned int quadVBO, quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    //Fill buffer with vertex data.
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    //Link vertex attributes (spacing between attributes).
    glBindVertexArray(quadVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
//HDR-framebuffer
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    //Create floating point (high-dynamic-range) color buffer (buffer can store float-values outside of the default range of [0.0, 1.0]).
    unsigned int hdrColorBuffer;
    glGenTextures(1, &hdrColorBuffer);

    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_W, SCR_H, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //Create depth buffer (renderbuffer).
    unsigned int hdrDepthBuffer;
    glGenRenderbuffers(1, &hdrDepthBuffer);

    glBindRenderbuffer(GL_RENDERBUFFER, hdrDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_W, SCR_H);

//Attach color -and depthbuffer to framebuffer (hdrFBO).
    //Color
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorBuffer, 0);
    //Depth
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrDepthBuffer);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        cout << "Framebuffer not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
/**...                                      */
/**'Static' transformation matrices...      */
    //Model
    glm::mat4 model=glm::mat4(1.0f);
    model=glm::translate(model, glm::vec3(0.0f, 0.0f, 25.0f));
    model=glm::scale(model, glm::vec3(2.5f, 2.5f, 27.5f));
    //Projection
    glm::mat4 projection=glm::mat4(1.0f);
    projection=glm::perspective(glm::radians(45.0f), SCR_W/SCR_H, 0.1f, 100.0f);
/**...                                      */
/**Set 'static' shader uniforms...          */
//Shader
    shader.use();
    //Sampler2D
    shader.setInt("diffuseTexture", 0);
    //Matrices
    shader.setMat4("model", model);
    shader.setMat4("projection", projection);
    //Light properties
    for(int n=0; n<lightCount; n++){
        shader.setVec3("light["+to_string(n)+"].position", lightPos[n]);
        shader.setFloat("light["+to_string(n)+"].ambient", 0.0f);
        shader.setVec3("light["+to_string(n)+"].diffuse", lightCol[n]);
        shader.setVec3("light["+to_string(n)+"].specular", lightCol[n]);
    }
//hdrShader
    hdrShader.use();
    //Sampler2D
    hdrShader.setInt("hdrBuffer", 0);
    //Other
    hdrShader.setBool("hdr", true);
    hdrShader.setFloat("exposure", exposure);
/**...                                      */
/**Last settings...                         */
//Depth buffer properties.
    glEnable(GL_DEPTH_TEST);
//Window & cursor.
    window.setFramerateLimit(60);
    sf::Mouse::setPosition(Vector2i(1920/2, 1080/2));
    window.setMouseCursorGrabbed(true);
//Variables.
    sf::Clock c;
    deltaTime=0.0f;
    bool run=true;
/**...                                      */
    while(run){
        //Measure time
        float currentFrame=c.getElapsedTime().asSeconds();
        deltaTime=currentFrame-lastFrame;
        lastFrame=currentFrame;
    //Process input
        run=processInput(window);
        moveCamera();
    //Clear framebuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Set shader's dynamic uniforms.
        shader.use();
        shader.setMat4("view", camera.GetViewMatrix());
        shader.setVec3("viewPos", camera.Position);
    //1. Render scene to floating-point-framebuffer (hdrFBO).
        shader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTex);
        renderCube(cubeVAO);

    //2. Render floating-point-colorbuffer to 2D-quad.
        hdrShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
        renderQuad(quadVAO);

        window.display();
    }
/**Delete rescoures...                      */

/**...                                      */
}
/***********************************************************************************************/
/***************************** MAIN FUNCTION END ***********************************************/
void renderCube(const unsigned int &cubeVAO){
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
void renderQuad(const unsigned int &quadVAO){
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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
Image loadImage(const char *path, bool flip){
    Image img;
    if(!img.loadFromFile(path)){
        cout << "Coud not load " << path << endl;
        return img;
    }
    if(flip) img.flipVertically();
    return img;
}
