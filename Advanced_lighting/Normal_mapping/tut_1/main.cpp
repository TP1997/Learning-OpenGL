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
unsigned int loadCubemap(const char **faces);
Image loadImage(const char *, bool);

void renderScene(const Shader &shader, const unsigned int &cubeVAO);
void renderCube(const unsigned int &cubeVAO);
void renderPlane(const unsigned int &planeVAO);
/***********************************************************************************************/
/***************************** MAIN FUNCTION START *********************************************/
int main(){
    sf::Window window(sf::VideoMode(SCR_W, SCR_H), "Tut_1_Point_shadows", sf::Style::Default, setSettings());
    checkOglVersion(window);
    glewInit();

/**Shader Build...                          */
    Shader shader("shaders/shader.vs", "shaders/shader.fs");
    Shader lampShader("shaders/lampShader.vs", "shaders/lampShader.fs");
/**...                                      */
/**Texture loading...                       */
    unsigned int brickwallTexture=loadTexture("images/brickwall.jpg", GL_REPEAT);
    unsigned int brickwallTextureNormal=loadTexture("images/brickwall_normal.jpg", GL_REPEAT);
/**...                                      */
/**Light properties...                      */
    glm::vec3 lightPos=glm::vec3(0.0f, 0.0f, 2.0f);
    glm::vec3 lightColor=glm::vec3(0.8f, 0.81f, 0.81f);
/**...                                      */
/**VBO/VAO/FBO/TBO...                       */
//Plane
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
    glBindVertexArray(0);
//Cube (lamp)
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
    glBindVertexArray(0);
/**...                                      */
/**Static transformation matrices...        */
    //Model
    glm::mat4 model=glm::mat4(1.0f);
    //Projection
    glm::mat4 projection=glm::mat4(1.0);
    projection=glm::perspective(glm::radians(45.0f), SCR_W/SCR_H, 0.1f, 100.0f);
/**...                                      */
/**Set 'static' shader uniforms...          */
//Shader
    shader.use();
    //Sampler2D
    shader.setInt("diffuseTexture", 0);
    shader.setInt("normalMap", 1);
    //Matrices
    shader.setMat4("model", model);
    shader.setMat4("projection", projection);
    //Lighting properties
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient", 0.3f*lightColor);
    shader.setVec3("light.diffuse", lightColor);
    shader.setVec3("light.specular", lightColor);
    //Other
    shader.setBool("useNormalMap", true);
//Lamp shader
    lampShader.use();
    //Matrices
    model=glm::mat4(1.0f);
    model=glm::translate(model, lightPos);
    model=glm::scale(model, glm::vec3(0.2f));
    lampShader.setMat4("model", model);
    lampShader.setMat4("projection", projection);
    //Light properties
    lampShader.setVec3("lightColor", lightColor);
/**...                                      */
/**Last settings...                         */
//Depth buffer properties.
    glEnable(GL_DEPTH_TEST);
//Face culling properties.
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BACK);
    glFrontFace(GL_CCW);
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

    //1. Set shader's 'dynamic' uniforms.
        glm::mat4 view=camera.GetViewMatrix();
        glm::vec3 viewPos=camera.Position;
        shader.use();
        shader.setMat4("view", view);
        shader.setVec3("viewPos", viewPos);
        lampShader.use();
        lampShader.setMat4("view", view);
        lampShader.setVec3("viewPos", viewPos);

    //2. Render plane
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickwallTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brickwallTextureNormal);
        renderPlane(planeVAO);
    //3. Render lamp as well (optional)
        lampShader.use();
        renderCube(cubeVAO);

        window.display();

    }
/**Delete rescoures...                      */

/**...                                      */
}
/***********************************************************************************************/
/***************************** MAIN FUNCTION END ***********************************************/
void renderScene(const Shader &shader, const unsigned int &cubeVAO){
    //room cube
 /*   glm::mat4 model=glm::mat4(1.0f);
    model=glm::scale(model, glm::vec3(5.0f));
    shader.setMat4("model", model);
    glDisable(GL_CULL_FACE);
    shader.setFloat("reverseFactor", -1.0f);
    renderCube(cubeVAO);
    shader.setFloat("reverseFactor", 1.0f);
    glEnable(GL_CULL_FACE);
    //cube1
    model=glm::mat4(1.0f);
    model=glm::translate(model, cubePosition[0]);
    model=glm::scale(model, cubeScale[0]);
    shader.setMat4("model", model);
    renderCube(cubeVAO);
    //cube2
    model=glm::mat4(1.0f);
    model=glm::translate(model, cubePosition[1]);
    model=glm::scale(model, cubeScale[1]);
    shader.setMat4("model", model);
    renderCube(cubeVAO);
    //cube3
    model=glm::mat4(1.0f);
    model=glm::translate(model, cubePosition[2]);
    model=glm::scale(model, cubeScale[2]);
    shader.setMat4("model", model);
    renderCube(cubeVAO);
    //cube4
    model=glm::mat4(1.0f);
    model=glm::translate(model, cubePosition[3]);
    model=glm::scale(model, cubeScale[3]);
    shader.setMat4("model", model);
    renderCube(cubeVAO);
    //cube5
    model=glm::mat4(1.0f);
    model=glm::translate(model, cubePosition[4]);
    model=glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    model=glm::scale(model, cubeScale[4]);
    shader.setMat4("model", model);
    renderCube(cubeVAO);*/

}
void renderCube(const unsigned int &cubeVAO){
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
void renderPlane(const unsigned int &planeVAO){
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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


