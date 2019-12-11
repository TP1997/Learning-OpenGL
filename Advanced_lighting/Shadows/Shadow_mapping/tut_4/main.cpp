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
const float WIDTH=800;
const float HEIGHT=800;
const unsigned int SHDW_W=1024;
const unsigned int SHDW_H=1024;

float lastX=WIDTH/2;
float lastY=HEIGHT/2;
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

void renderScene(const Shader &shader, unsigned int &planeVAO, unsigned int &cubeVAO);
void renderQuad(unsigned int &quadVAO);
void renderCube(unsigned int &cubeVAO);
/***********************************************************************************************/
int main(){
    sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "Tut_2", sf::Style::Default, setSettings());
    checkOglVersion(window);
    glewInit();

/**Shader Build...                          */
    Shader shader("shaders/shader.vs", "shaders/shader.fs");
    Shader depthShader("shaders/depthShader.vs", "shaders/depthShader.fs");;
/**...                                      */
/**Texture loading...                       */
    unsigned int woodTexture=loadTexture("images/wood.jpg", GL_REPEAT);

/**...                                      */
/**Light space transformation...            */
    float nPlane=1.0f, fPlane=7.5f;
    glm::vec3 lightPos=glm::vec3(-2.0f, 4.0f, -1.0f);
    //glm::vec3 lightPos=glm::vec3(0.010f, 4.0f, 0.0f);
    glm::mat4 lightProjection=glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nPlane, fPlane);
    glm::mat4 lightView=glm::lookAt(lightPos                     ,      //Position
                                    glm::vec3( 0.0f, 0.0f,  0.0f),      //Traget
                                    glm::vec3( 0.0f, 1.0f,  0.0f));     //Up

    glm::mat4 lightSpaceMatrix=lightProjection*lightView;
/**...                                      */

/**VBO/VAO/FBO/TBO...                       */
//Cube VBO/VAO
    unsigned int cubeVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
//Plane vertex VBO/VAO
    unsigned int planeVBO, planeVAO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
//Depth map FBO (shadow mapping)
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    //Texture representing framebuffers (depthMapFBO) depth map
    unsigned int depthMap;
    glGenTextures(1, &depthMap);

    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHDW_W, SHDW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);      //GL_CLAMP_TO_EDGE, works as well
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);      //GL_CLAMP_TO_EDGE --""--
    //All coordinates outside depth map's range have depth of 1.0
    //--> these coordinates will never be in shadow.
    float borderColor[]={1.0f, 1.0f, 1.0f, 1.0f};                               //If ^, then not needed
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);      //If ^^, then not needed
    //Attach as the framebuffer's (depthMapFBO) depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
/**...                                  */
/**Perspective projection matrix...     */
    glm::mat4 projection=glm::mat4(1.0);
    projection=glm::perspective(glm::radians(45.0f), WIDTH/HEIGHT, 0.1f, 100.0f);
/**...                                 */
/**Set 'static' shader uniforms...     */
//Shader
    shader.use();
    //Sampler2D
    shader.setInt("diffuseTexture", 0);
    shader.setInt("shadowMap", 1);
    //Matrices
    shader.setMat4("projection", projection);
    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    //Lightning attributes
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient", glm::vec3(0.3f, 0.3f, 0.3f));
    shader.setVec3("light.diffuse", glm::vec3(0.3f, 0.3f, 0.3f));
    shader.setVec3("light.specular", glm::vec3(0.3f, 0.3f, 0.3f));
//Depth shader
    depthShader.use();
    //Matrices
    depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
/**...                              */
/**Last settings...                 */
    glEnable(GL_DEPTH_TEST);
//Face culling properties
    glEnable(GL_CULL_FACE);
    glEnable(GL_BACK);
    glFrontFace(GL_CCW);
//Window & cursor
    window.setFramerateLimit(60);
    sf::Mouse::setPosition(Vector2i(1920/2, 1080/2));
    window.setMouseCursorGrabbed(true);
//Variables
    sf::Clock c;
    deltaTime=0.0f;
    bool run=true;
/**...                              */
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
    //1. Render depth of the scene to texture (from light's perspective).
        depthShader.use();
        glViewport(0, 0, SHDW_W, SHDW_H);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        //Culling front faces removes peter panning completily (check also fragment shader).
        //glCullFace(GL_FRONT);
        renderScene(depthShader, planeVAO, cubeVAO);
        //glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //2. Render scene as normal using the generated depth/shadow map.
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        glm::mat4 view=camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderScene(shader, planeVAO, cubeVAO);

        window.display();

    }
/**Delete rescoures...              */

/**...                              */
}

/***********************************************************************************************/
void renderScene(const Shader &shader, unsigned int &planeVAO, unsigned int &cubeVAO){
    //floor
    glm::mat4 model=glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //cube1
    model=glm::mat4(1.0f);
    model=glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
    model=glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube(cubeVAO);
    //cube2
    model=glm::mat4(1.0f);
    model=glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0f));
    model=glm::scale(model, glm::vec3(0.9f));
    shader.setMat4("model", model);
    renderCube(cubeVAO);
    //cube3
    model=glm::mat4(1.0f);
    model=glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0f));
    model=glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    model=glm::scale(model, glm::vec3(0.25f));
    shader.setMat4("model", model);
    renderCube(cubeVAO);

}
void renderCube(unsigned int &cubeVAO){
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
void renderQuad(unsigned int &quadVAO){
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
