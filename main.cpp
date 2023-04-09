#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightColor;

//light
const glm::vec3 lightPos = glm::vec3(-512.442f, 366.83f, 517.685f);
const glm::vec3 lightTarget = glm::vec3(591.658f, 8.5229f, - 591.419f);
//const glm::vec3 lightPos = glm::vec3(-584.158f, 406.506f, -571.987f);
//const glm::vec3 lightTarget = glm::vec3(591.764f, 5.02233f, 595.648f);
const glm::vec3 lightDir = (lightPos - lightTarget);

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

// camera
//Camera myCamera(glm::vec3(0.0f, 0.0f, 3.0f));

//GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D door;
gps::Model3D scene;
gps::Model3D car;

// shaders
gps::Shader myBasicShader;
gps::Shader screenQuadShader;
gps::Model3D screenQuad;
gps::Shader skyBox;
// variables
int glWindowWidth = 1024;
int glWindowHeight = 768;
Camera myCamera(glm::vec3(350.0f, 50.0f, 250.0f));
// sa fie aceeasi viteza WASD
float currentFrame, deltaTime = 0.0f, lastFrame = 0.0f, cameraSpeed;
// pt mouse
float lastX = glWindowWidth / 2.0f;
float lastY = glWindowHeight / 2.0f;
bool firstMouse = true;

//shadows
GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap;
gps::Shader depthMapShader;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

//fog density
float fogDen = 0.0f;

//skybox
gps::SkyBox mySkyBox;

//animations
//door
float unghi = 0.0f;
bool door_closed = true, door_dir = true, door_opened=false;

//car
float sx = -33.6451f, sy = 6.68968f, sz = 545.47f;
float cx = 0.0f, cy = 0.0f, cz = 0.0f;
float fx = 45.1311f, fy = 6.68968f, fz = -1000.0f;
bool go = false;

// presentation animation
bool stage[10];
bool animate = false;
double presentation_start;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    myCamera.ProcessMouseMovement(xoffset, yoffset);
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // depth map
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    // usa 
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        door_closed = false;
        door_opened = false;
        door_dir = !door_dir;
    }

    // masina
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        go = !go;

    //ceata
    if (key == GLFW_KEY_3)
    {
        if (fogDen >= 0.001f)
            fogDen -= 0.001;
        myBasicShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity"), fogDen);
    }

    if (key == GLFW_KEY_4)
    {
        if (fogDen <= 0.003f)
            fogDen += 0.001;
        myBasicShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity"), fogDen);
    }

    //wireframe
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    //punctiforma
    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    // prezentare
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        animate = !animate;
        if (animate) {
            // init animation
            glfwSetCursorPosCallback(myWindow.getWindow(), NULL);
            myCamera.initAnimation();
            for (int i = 1; i < 10; i++) {
                stage[i] = true;
            }
            presentation_start = glfwGetTime();
        }
        else {
            //end animation
            glfwSetCursorPosCallback(myWindow.getWindow(), mouse_callback);
            myCamera.endAnimation();
        }
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}


void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.ProcessKeyboard(MOVE_FORWARD, deltaTime);
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.ProcessKeyboard(MOVE_BACKWARD, deltaTime);
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.ProcessKeyboard(MOVE_LEFT, deltaTime);
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.ProcessKeyboard(MOVE_RIGHT, deltaTime);
    }
}


void initOpenGLWindow() {
    myWindow.Create(glWindowWidth, glWindowHeight, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouse_callback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	//glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBO() {
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 2000.0f;
    // recalculare modelululi, se inmulteste matricea modelului cu ...

    glm::mat4 lightProjection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, near_plane, far_plane);
    //glm::mat4 lightSpaceTrMatrix = lightProjection * lightView * (-model);
    //model = glm::translate(model, glm::vec3(0.0f, 50.0f, 0.0f));
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void initModels() {
    //teapot.LoadModel("models/teapot/teapot20segUT.obj");
    scene.LoadModel("models/full_scene/full_scene.obj");
    door.LoadModel("models/door/door.obj");
    car.LoadModel("models/speedcar/speed_car.obj");
    //door1.LoadModel("models/door/door1.obj");
}

void initSkyBox() {
    std::vector<const GLchar*> faces;
    /*faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");*/
    faces.push_back("skybox/posx.jpg");
    faces.push_back("skybox/negx.jpg");
    faces.push_back("skybox/posy.jpg");
    faces.push_back("skybox/negy.jpg");
    faces.push_back("skybox/posz.jpg");
    faces.push_back("skybox/negz.jpg");
    mySkyBox.Load(faces);
}

void initShaders() {
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    myBasicShader.useShaderProgram();
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
    depthMapShader.useShaderProgram();
    skyBox.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyBox.useShaderProgram();
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 2000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(normalMatrix * lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity"), 0.0f);
}

void drawObjects(gps::Shader shader, bool depthPass) {

    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    scene.Draw(shader);

    model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    if (!door_closed && !door_opened) {
        if (door_dir) {
            unghi += deltaTime * 100.0f;
        }
        else {
            unghi -= deltaTime * 100.0f;
        }
    }
    
    if (unghi < -89.0 && !door_dir)
    {
        door_closed = true;
        unghi = -90.0f;
    }
    if (unghi > -1.0 && door_dir) 
    {
        door_opened = true;
        unghi = 0.0f;
    }

    model = glm::translate(model, glm::vec3(364.411f, 29.6912f, 2.52379f));
    model = glm::rotate(model, glm::radians(unghi), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-364.411f, -29.6912f, -2.52379f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    door.Draw(shader);

    model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    if (go) {
        if (cx <= fx)
            //cx = cx + (deltaTime * 100.0f)/(fx-sx);
            cx = cx + deltaTime * 5.0f;
        if (cz >= fz)
            //cz = cz + (deltaTime * 100.0f)/(fz-sz);
            cz = cz - deltaTime * 100.0f;
        if (cz <= fz) {
            model = glm::mat4(1.0f);
            cx = 0.0f;  cz = 0.0f;
        }
        else
            model = glm::translate(model, glm::vec3(cx, 0.0f, cz));
    }

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    car.Draw(shader);

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyBox.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    //skybox
    projection = glm::perspective(glm::radians(45.0f), (float) myWindow.getWindowDimensions().width / myWindow.getWindowDimensions().width, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyBox.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    mySkyBox.Draw(skyBox, view, projection);
}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}

void renderFullScene(gps::Shader shader) {
    model = glm::mat4(1.0f);
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    scene.Draw(shader);
}

void renderDoor(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(320.0, -20.0, 4.0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    door.Draw(shader);
}

void renderScene() {
    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    cameraSpeed = 5.0f * deltaTime;

    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (showDepthMap) {
        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        //bind the depth map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        myBasicShader.useShaderProgram();
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * model)) * lightDir));

        //bind the shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(computeLightSpaceTrMatrix()));

        drawObjects(myBasicShader, false);

        if (animate && glfwGetTime() > presentation_start + 0.01) {
            if (glfwGetTime() < presentation_start + 5 && stage[1]) {
                myCamera.ProcessKeyboard(MOVE_FORWARD, deltaTime);
            }
            else if (glfwGetTime() < presentation_start + 8 && stage[2]) {
                stage[1] = false;
                myCamera.ProcessMouseMovement(4.0f, 0.0f);
            }else if (glfwGetTime() < presentation_start + 11 && stage[3]) {
                stage[2] = false;
                myCamera.ProcessKeyboard(MOVE_FORWARD, deltaTime);
            }else if (glfwGetTime() < presentation_start + 16 && stage[4]) {
                stage[3] = false;
                myCamera.ProcessMouseMovement(-1.0f, 0.0f);
            }
        }
    }

    //model = glm::mat4(1.0f);
    //glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    //glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model)) * lightDir));
}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    initFBO();
    initSkyBox();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        renderScene();
        processMovement();
	    

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		//glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
