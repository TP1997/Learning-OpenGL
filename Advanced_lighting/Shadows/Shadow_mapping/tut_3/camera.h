#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Movement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const uint8_t wmask=1<<0;
const uint8_t smask=1<<1;
const uint8_t amask=1<<2;
const uint8_t dmask=1<<3;

const float YAW=-90.0f;
const float PITCH=0.0f;
const float SPEED=4.0f;
const float SENSITIVITY=0.005f;
const float ZOOM=45.0f;

class Camera{
    public:

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position=glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f), float yaw=YAW, float pitch=PITCH);
    Camera(float, float, float, float, float, float, float, float);

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(const uint8_t, float);
    void ProcessMouseMovement(float, float, GLboolean);
    void ProcessMouseScroll(float);

    private:
    void updateCameraVectors();



};

#endif // CAMERA_H_INCLUDED
