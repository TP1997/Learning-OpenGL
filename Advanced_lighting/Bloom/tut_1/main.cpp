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
    Shader hdrCalcShader("shaders/hdrCalcShader.vs", "shaders/hdrCalcShader.fs");
    Shader gaussianBlurShader("shaders/gaussianBlurShader.vs", "shaders/gaussianBlurShader.fs");
    Shader lampShader("shaders/hdrCalcShader.vs", "shaders/lampShader.fs");
    Shader finalShader("shaders/quadToneMapShader.vs", "shaders/quadToneMapShader.fs");
/**...                                      */
/**Texture loading...                       */
    unsigned int containerTex=loadTexture("images/container2.png", GL_REPEAT);
    unsigned int containerTexSpec=loadTexture("images/container2_specular.png", GL_REPEAT);
    unsigned int woodFloorTex=loadTexture("images/wood.png", GL_REPEAT);
/**...                                      */
/**Light properties...                      */
    float exposure=1.0f;
    int lightCount=4;
    glm::vec3 lightPos[]={ glm::vec3( 0.0f, 0.5f,  1.5f),
                           glm::vec3(-4.0f, 5.5f, -3.0f),
                           glm::vec3( 3.0f, 0.5f,  1.0f),
                           glm::vec3(-0.8f, 2.4f, -1.0f) };
    //Light colors (high-dynamic-range).
    glm::vec3 lightCol[]={ glm::vec3(5.0f,  5.0f, 5.0f),
                           glm::vec3(10.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f,  0.0f, 15.0f),
                           glm::vec3(0.0f,  5.0f, 0.0f) };
/**...                                      */
/**Cube properties                          */
    glm::vec3 cubePos[]={ glm::vec3( 0.0f,  1.5f,  0.0f),
                          glm::vec3( 2.0f,  0.0f,  1.0f),
                          glm::vec3(-1.0f, -1.0f,  2.0f),
                          glm::vec3( 0.0f,  2.7f,  4.0f),
                          glm::vec3(-2.0f,  1.0f, -3.0),
                          glm::vec3(-3.0f,  0.0f,  0.0) };

    glm::vec3 cubeScale[]={ glm::vec3(0.5f),
                            glm::vec3(0.5f),
                            glm::vec3(1.0f),
                            glm::vec3(1.25f),
                            glm::vec3(1.0f),
                            glm::vec3(0.5f) };

    float cubeRotation[]={0.0f, 0.0f, 60.0f, 23.0f, 124.0f, 0.0f};
/**                                         */
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
//Texture quad
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
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    //Create floating point (high-dynamic-range) color buffers (buffer can store float-values outside of the default range of [0.0, 1.0]).
    //Two colorbuffers (GL_COLOR_ATTACHMENT0 for whole scene, GL_COLOR_ATTACHMENT1 for bright fragments only).
    unsigned int hdrColorBuffer[2];
    glGenTextures(2, hdrColorBuffer);
    //Generate texture images and set parameters for both color buffers.
    for(unsigned int n=0; n<2; n++){
        glBindTexture(GL_TEXTURE_2D, hdrColorBuffer[n]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_W, SCR_H, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //Attach texture to framebuffer.
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+n, GL_TEXTURE_2D, hdrColorBuffer[n], 0);
    }
    //Create depth buffer (renderbuffer).
    unsigned int hdrDepthBuffer;
    glGenRenderbuffers(1, &hdrDepthBuffer);

    glBindRenderbuffer(GL_RENDERBUFFER, hdrDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_W, SCR_H);
    //Attach depthbuffer to framebuffer (hdrFBO).
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrDepthBuffer);
    //Tell openGL which color attachment's we'll use for rendering.
    unsigned int attachments[2]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    //Finally, check if framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        cout << "Framebuffer not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

//Ping-pong framebuffer for blurring.
    unsigned int pingpongFBO[2];
    glGenFramebuffers(2, pingpongFBO);
    //Create floating point (high-dynamic-range) color buffers (buffer can store float-values outside of the default range of [0.0, 1.0]).
    //Buffers for horizontal & vertical blurring.
    unsigned int pinpongColorBuffer[2];
    glGenTextures(2, pinpongColorBuffer);
    //Generate texture images and set parameters for both color buffers.
    for(unsigned int n=0; n<2; n++){
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[n]);
        glBindTexture(GL_TEXTURE_2D, pinpongColorBuffer[n]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_W, SCR_H, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //Attach texture to framebuffer.
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pinpongColorBuffer[n], 0);//hdrColorBuffer[1]
        //Check if framebuffer is complete.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cout << "Framebuffer not complete!" << endl;
    }

/**...                                      */
/**'Static' transformation matrices...      */
    //Model
    glm::mat4 model=glm::mat4(1.0f);
    //Projection
    glm::mat4 projection=glm::mat4(1.0f);
    projection=glm::perspective(glm::radians(45.0f), SCR_W/SCR_H, 0.1f, 100.0f);
/**...                                      */
/**Set 'static' shader uniforms...          */
//hdrCalcShader
    hdrCalcShader.use();
    //Sampler2D
    hdrCalcShader.setInt("diffuseTexture", 0);
    //Matrices
    hdrCalcShader.setMat4("model", model);
    hdrCalcShader.setMat4("projection", projection);
    //Light properties
    for(int n=0; n<lightCount; n++){
        hdrCalcShader.setVec3("light["+to_string(n)+"].position", lightPos[n]);
        hdrCalcShader.setFloat("light["+to_string(n)+"].ambient", 0.1f);
        hdrCalcShader.setVec3("light["+to_string(n)+"].diffuse", lightCol[n]);
        hdrCalcShader.setVec3("light["+to_string(n)+"].specular", lightCol[n]);
    }
//gaussianBlurShader
    gaussianBlurShader.use();
    //Sampler2D
    gaussianBlurShader.setInt("image", 0);
//lampShader
    lampShader.use();
    //Matrices
    lampShader.setMat4("projection", projection);
//finalShader
    finalShader.use();
    //Sampler2D
    finalShader.setInt("hdrBuffer", 0);
    finalShader.setInt("gaussianBlur", 1);
    //Other
    finalShader.setFloat("exposure", exposure);
    finalShader.setBool("bloom", true);

/**...                                      */
/**Last settings...                         */
//Depth buffer properties.
    glEnable(GL_DEPTH_TEST);
//Face culling

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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Set shader's dynamic uniforms.
        hdrCalcShader.use();
        hdrCalcShader.setMat4("view", camera.GetViewMatrix());
        hdrCalcShader.setVec3("viewPos", camera.Position);
        lampShader.use();
        lampShader.setMat4("view", camera.GetViewMatrix());
        lampShader.setVec3("viewPos", camera.Position);

    //1. Render scene to floating-point-framebuffer (hdrFBO).
        hdrCalcShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Render floor.
        model=glm::mat4(1.0f);
        model=glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model=glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
        hdrCalcShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodFloorTex);
        renderCube(cubeVAO);
        //Render cubes.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerTex);
        for(unsigned int n=0; n<6; n++){
            model=glm::mat4(1.0f);
            model=glm::translate(model, cubePos[n]);
            model=glm::rotate(model, glm::radians(cubeRotation[n]), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
            model=glm::scale(model, cubeScale[n]);
            hdrCalcShader.setMat4("model", model);
            renderCube(cubeVAO);
        }
        //Render lights.
        lampShader.use();
        for(unsigned int n=0; n<4; n++){
            model=glm::mat4(1.0f);
            model=glm::translate(model, lightPos[n]);
            model=glm::scale(model, glm::vec3(0.25f));
            lampShader.setMat4("model", model);
            lampShader.setVec3("lightColor", lightCol[n]);
            renderCube(cubeVAO);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //2. Blur bright fragments with two-pass Gaussian Blur
        gaussianBlurShader.use();
        bool horizontal=true;
        unsigned int amount=10;
        //First bind & blur & render the texture we'd like to blur. Otherwise the pingpongColorBuffers will end up empty.
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        gaussianBlurShader.setBool("horizontal", horizontal);
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrColorBuffer[1]);
        renderQuad(quadVAO);
        horizontal=!horizontal;
        //Repeat blurring process amount-1 times.
        for(unsigned int n=1; n<amount; n++){
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            gaussianBlurShader.setBool("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, pinpongColorBuffer[!horizontal]);//hdrColorBuffer[1]
            renderQuad(quadVAO);
            horizontal=!horizontal;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //3. Render floating-point-colorbuffer (hdr) to 2D-quad.
        finalShader.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Bind scene's hdr-texture.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrColorBuffer[0]);
        //Bind scene's blurred brightness texture.
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pinpongColorBuffer[!horizontal]);//hdrColorBuffer[1]
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
