#include "shader.h"
#include "vertices.h"
#include "camera.h"
#include "model.h"

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
/**Shader build...                          */
    Shader geometryPassShader("shaders/geometryPassShader.vs", "shaders/geometryPassShader.fs");
    Shader lightingPassShader("shaders/lightingPassShader.vs", "shaders/lightingPassShader.fs");
    Shader lampShader("shaders/lampShader.vs", "shaders/lampShader.fs");
/**...                                      */
/**Model loading...                       */
unsigned int wood=loadTexture("images/wood.png", GL_REPEAT);

    Model nanosuit("models/nanosuit/nanosuit.obj");
    const unsigned int objectCount=9;
    glm::vec3 objectPositions[]={ glm::vec3(-3.0,  -3.0, -3.0),
                                  glm::vec3( 0.0,  -3.0, -3.0),
                                  glm::vec3( 3.0,  -3.0, -3.0),
                                  glm::vec3(-3.0,  -3.0,  0.0),
                                  glm::vec3( 0.0,  -3.0,  0.0),
                                  glm::vec3( 3.0,  -3.0,  0.0),
                                  glm::vec3(-3.0,  -3.0,  3.0),
                                  glm::vec3( 0.0,  -3.0,  3.0),
                                  glm::vec3( 3.0,  -3.0,  3.0) };
/**...                                      */
/**Light properties...                      */
    const unsigned int lightCount=32;
    const float lampScale=0.125f;
    const float linAttenuation=0.7f;
    const float quadAttenuation=1.8f;
    glm::vec3 lightPos[lightCount];
    glm::vec3 lightCol[lightCount];
    srand(time(nullptr));
    for(unsigned int n=0; n<lightCount; n++){
        //Calculate random offsets.
        //srand(13);
        float xPos = ((rand() % 100) / 100.0f) * 6.0 - 3.0;
        float yPos = ((rand() % 100) / 100.0f) * 6.0 - 4.0;
        float zPos = ((rand() % 100) / 100.0f) * 6.0 - 3.0;
        lightPos[n]=glm::vec3(xPos, yPos, zPos);
        //Calculate random color.
        float rCol=((rand()%100)/200.0f)+0.5f;  //random value [0.5, 1.0].
        float gCol=((rand()%100)/200.0f)+0.5f;  //random value [0.5, 1.0].
        float bCol=((rand()%100)/200.0f)+0.5f;  //random value [0.5, 1.0].
        lightCol[n]=glm::vec3(rCol, gCol, bCol);
    }
/**...                                      */
/**VBO/VAO/FBO/TBO...                       */
//Cube (for lamp objects).
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

//Quad (screen).
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

//G-buffer.
    //Create custom framebuffer.
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    //Create colorbuffers for G-buffer.
    unsigned int gPosition, gNormal, gAlbedoSpec;
    //Position texture.
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_W, SCR_H, 0, GL_RGB, GL_FLOAT, NULL);             //High precision.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);      //Attach to framebuffer.

    //Normal texture.
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_W, SCR_H, 0, GL_RGB, GL_FLOAT, NULL);             //High precision.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);        //Attach to framebuffer.

    //Diffuse & specular texture.
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_W, SCR_H, 0, GL_RGBA, GL_FLOAT, NULL);               //Low precision (A for specular intensity).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);    //Attach to framebuffer.

    //Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering.
    unsigned int colorAttachments[]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, colorAttachments);

    //Create also render (depth) buffer
    unsigned int gDepth;
    glGenRenderbuffers(1, &gDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_W, SCR_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepth);        //Attach to framebuffer.

    //Check framebuffer completiness.
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        cout <<"Framebuffer not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

/**...                                      */
/**'Static' transformation matrices...      */
    glm::mat4 projection=glm::perspective(glm::radians(45.0f), SCR_W/SCR_H, 0.1f, 100.0f);
/**...                                      */
/**Set 'static' shader uniforms...          */
//geometryPassShader
    geometryPassShader.use();
    //Matrices.
    geometryPassShader.setMat4("projection", projection);
    //Sampler2D.
    geometryPassShader.setInt("texture_diffuse1", 0);
    geometryPassShader.setInt("texture_specular1", 1);

//lightingPassShader.
    lightingPassShader.use();
    //Sampler2D (initalize G-buffer rescoures).
    lightingPassShader.setInt("gPosition", 0);
    lightingPassShader.setInt("gNormal", 1);
    lightingPassShader.setInt("gAlbedoSpec", 2);
    //Light properties.
    for(unsigned int n=0; n<lightCount; n++){
        lightingPassShader.setVec3("light["+to_string(n)+"].position", lightPos[n]);
        lightingPassShader.setFloat("light["+to_string(n)+"].ambient", 0.1f);
        lightingPassShader.setVec3("light["+to_string(n)+"].diffuse", lightCol[n]);
        lightingPassShader.setVec3("light["+to_string(n)+"].specular", lightCol[n]);
        lightingPassShader.setFloat("light["+to_string(n)+"].linear", linAttenuation);
        lightingPassShader.setFloat("light["+to_string(n)+"].quadratic", quadAttenuation);
    }

//lampShader
    lampShader.use();
    //Matrices.
    lampShader.setMat4("projection", projection);

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
    //Measure time.
        float currentFrame=c.getElapsedTime().asSeconds();
        deltaTime=currentFrame-lastFrame;
        lastFrame=currentFrame;
    //Process input.
        run=processInput(window);
        moveCamera();
    //Clear framebuffer.
        glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Get dynamic uniforms
        glm::mat4 model=glm::mat4(1.0f);
        glm::mat4 view=camera.GetViewMatrix();
        glm::vec3 viewPos=camera.Position;

    //1. Geometry-pass: Render scene's geometry & color data to G-buffer.
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wood);
        //Uniform setting & rendering.
        geometryPassShader.use();
        geometryPassShader.setMat4("view", view);
        for(unsigned int n=0; n<objectCount; n++){
            model=glm::mat4(1.0f);
            model=glm::translate(model, objectPositions[n]);
            model=glm::scale(model, glm::vec3(0.25f));
            geometryPassShader.setMat4("model", model);
            nanosuit.draw(geometryPassShader);
        }

    //2. Lighting-pass: Calculate scene's lighting by iterating over a screen-filled quad using information stored in G-buffer.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);               //Position colorbuffer (G-buffer).
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);               //Normal colorbuffer (G-buffer).
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);               //Albedo+specular colorbuffer (G-buffer).
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        //Uniform setting & rendering.
        lightingPassShader.use();
        lightingPassShader.setVec3("viewPos", viewPos);
        renderQuad(quadVAO);

    //2.5 Take screen-space depth information into account before rendering the lamps. Copy G-buffer's depthbuffer into default framebuffer's depthbuffer.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SCR_W, SCR_H, 0, 0, SCR_W, SCR_H, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    //3. Render light objects (lamps).
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //Uniform setting & rendering.
        lampShader.use();
        lampShader.setMat4("view", view);
        for(unsigned int n=0; n<lightCount; n++){
            model=glm::mat4(1.0f);
            model=glm::translate(model, lightPos[n]);
            model=glm::scale(model, glm::vec3(lampScale));
            lampShader.setMat4("model", model);
            lampShader.setVec3("lightColor", lightCol[n]);
            renderCube(cubeVAO);
        }

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
