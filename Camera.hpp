#ifndef Camera_hpp
#define Camera_hpp

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include <GLFW/glfw3.h>
#include <string>
    
enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT};
    
// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 200.0f;
const float SENSITIVITY = 0.1f;

class Camera
{
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
    glm::mat4 getViewMatrix();
    void ProcessKeyboard(MOVE_DIRECTION direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void updateCameraVectors();

    void initAnimation();
    void endAnimation();
    void setMovementSpeed(float movementSpeed);
    float getYaw();
    glm::vec3 getPosition();
    void setMouseSensitivity(float sensitivity);
private:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
};

#endif /* Camera_hpp */
