#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch): Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM){
    Position=position;
    WorldUp=up;
    Yaw=yaw;
    Pitch=pitch;
    updateCameraVectors();
}
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch): Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM){
    Position=glm::vec3(posX, posY, posZ);
    WorldUp=glm::vec3(upX, upY, upZ);
    Yaw=yaw;
    Pitch=pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix(){
    return glm::lookAt(Position, Position+Front, Up);
}
void Camera::ProcessKeyboard(const uint8_t bitmask, float deltaTime){
    float velocity=deltaTime*MovementSpeed;
    if(bitmask&wmask)
        Position+=Front*velocity;
    if(bitmask&smask)
        Position-=Front*velocity;
    if(bitmask&amask)
        Position-=Right*velocity;
    if(bitmask&dmask)
        Position+=Right*velocity;
}
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch=true){
    xoffset*=MouseSensitivity;
    yoffset*=MouseSensitivity;

    Yaw+=xoffset;
    Pitch+=yoffset;

    if(constrainPitch){
        if(Pitch>89.0f)
            Pitch=89.0f;
        if(Pitch<-89.0f)
            Pitch=-89.0f;
    }
    updateCameraVectors();
}
void Camera::ProcessMouseScroll(float yoffset){
    if(Zoom>=1.0f && Zoom<=45.0f)
        Zoom-=yoffset;
    if(Zoom<=1.0f)
        Zoom=1.0f;
    if(Zoom>=45.0f)
        Zoom=45.0f;
}

void Camera::updateCameraVectors(){
    glm::vec3 front;
    front.x=cos(Pitch)*cos(Yaw);
    front.y=sin(Pitch);
    front.z=cos(Pitch)*sin(Yaw);
    Front=glm::normalize(front);

    Right=glm::normalize(glm::cross(Front, WorldUp));
    Up=glm::normalize(glm::cross(Right, Front));

}
