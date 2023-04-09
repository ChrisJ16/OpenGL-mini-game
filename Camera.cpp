#include "Camera.hpp"
#include <GLFW\glfw3.h>
#include "glm/glm.hpp"
#include <GLFW\glfw3.h>
#include <iostream>
// constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}
// constructor with scalar values
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY){
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::getViewMatrix(){
    return glm::lookAt(Position, Position + Front, Up);
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::ProcessKeyboard(MOVE_DIRECTION direction, float deltaTime){
    float velocity = MovementSpeed * deltaTime;
    if (direction == MOVE_FORWARD)
        Position += Front * velocity;
    if (direction == MOVE_BACKWARD)
        Position -= Front * velocity;
    if (direction == MOVE_LEFT)
        Position -= Right * velocity;
    if (direction == MOVE_RIGHT)
        Position += Right * velocity;
    /*std::cout << this->Position.x << " " << this->Position.y << " " << this->Position.z << std::endl;
    glm::vec3 frontVec(Position + Front);
    std::cout << frontVec.x << " " << frontVec.y << " " << frontVec.z << std::endl;
    std::cout << "------------------------------" << std::endl;*/
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch){
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

// calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors(){
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::initAnimation() {
    Position = glm::vec3(-30.8829f, 28.5826f, 563.463f);
    Yaw = -90.0f;
    Pitch = 0.0f;
    MovementSpeed = 100.0f;
    updateCameraVectors();
}

void Camera::endAnimation() {
    MovementSpeed = SPEED;
    MouseSensitivity = SENSITIVITY;
}

void Camera::setMovementSpeed(float movementSpeed) {
    MovementSpeed = movementSpeed;
}

float Camera::getYaw() {
    return Yaw;
}

glm::vec3 Camera::getPosition() {
    return Position;
}

void Camera::setMouseSensitivity(float sensitivity) {
    MouseSensitivity = sensitivity;
}