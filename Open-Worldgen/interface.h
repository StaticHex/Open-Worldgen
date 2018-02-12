#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <math.h>

// OpenGL library includes
#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>

// Project includes
#include "Occulus.h"
#include "Camera.h"
#include "debuggl.h"

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::uvec3;
using glm::perspective;
using glm::radians;
using std::cout;
using std::endl;
using std::numeric_limits;

// Define statements
#define PRESS_W key == GLFW_KEY_W && action != GLFW_RELEASE
#define PRESS_A key == GLFW_KEY_A && action != GLFW_RELEASE
#define PRESS_S key == GLFW_KEY_S && action != GLFW_RELEASE
#define PRESS_D key == GLFW_KEY_D && action != GLFW_RELEASE
#define PRESS_LSHIFT key == GLFW_KEY_LEFT_SHIFT && action != GLFW_RELEASE
#define PRESS_RSHIFT key == GLFW_KEY_RIGHT_SHIFT && action != GLFW_RELEASE
#define PRESS_RCTRL key == GLFW_KEY_RIGHT_CONTROL && action != GLFW_RELEASE
#define PRESS_SPACE key == GLFW_KEY_SPACE && action != GLFW_RELEASE
#define PRESS_LEFT key == GLFW_KEY_LEFT && action != GLFW_RELEASE
#define PRESS_RIGHT key == GLFW_KEY_RIGHT && action != GLFW_RELEASE
#define PRESS_UP key == GLFW_KEY_UP && action != GLFW_RELEASE
#define PRESS_DOWN key == GLFW_KEY_DOWN && action != GLFW_RELEASE
#define PRESS_MIDDLE_BUTTON currentButton == GLFW_MOUSE_BUTTON_MIDDLE || currentButton == GLFW_MOUSE_BUTTON_8

// Defined constants
const char* winTitle = "Open Worldgen v1.0";

// Window Variables
int winWidth = 800;
int winHeight = 800;
bool isGlobal = true;
int currentButton;
bool mousePressed;
float aspect = 0.0f;

// Mouse position vectors
vec2 cPos = vec2(0, 0);
vec2 lPos = vec2(0, 0);
vec2 dV = vec2(0, 0);
float dVl = glm::length(dV);

// Terrain Shader Variables
vector<vec4> tVertices;
vector<float> tTemps;
vector<vec4> tNormals;
vector<uvec3> tFaces;
vector<vec2> tUv;

// Create the camera
Camera camera = Camera();

// upper and lower bounds
vec4 min_bounds = vec4(-(numeric_limits<float>::max)());
vec4 max_bounds = vec4((numeric_limits<float>::max)());

// Shader constants
vec4 lightPos = vec4(10.0f, 100.0f, 0.0f, 1.0f);
const float ambConstant = 0.10f;
const vec4 tAmbient = vec4(1.0f, 1.0f, 0.875f, 1.0f);
const vec4 tSpecular = vec4(0.01f, 0.01f, 0.01f, 1.0f);
const float tShininess = 0.01f;

// VBO and VAO descriptors
enum { kVertexBuffer, kNormalBuffer, kTempBuffer, kIndexBuffer, kNumVbos };

// VAOs
enum { kGeometryVao, kNumVaos };

//Variables to hold VAO and VBO descriptors
GLuint gArrayObjects[kNumVaos]; // Holds VAO descriptors
GLuint gBufferObjects[kNumVaos][kNumVbos]; // Holds VBO descriptors

// Function Prototypes

// Begin key callbacks and camera controls
void KeyCallback(GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (PRESS_W) {
			camera.setEye(camera.getEye() +
				camera.zoomSpeed*camera.getLook());
			camera.setCenter(camera.getCenter() +
				camera.zoomSpeed*camera.getLook());
	}
	else if (PRESS_S) {
			camera.setEye(camera.getEye() -
				camera.zoomSpeed*camera.getLook());
			camera.setCenter(camera.getCenter() -
				camera.zoomSpeed*camera.getLook());
	}
	else if (PRESS_A) {
			camera.setEye(camera.getEye() -
				camera.panSpeed*camera.getRight());
			camera.setCenter(camera.getCenter() -
				camera.panSpeed*camera.getRight());
	}
	else if (PRESS_D) {
			camera.setEye(camera.getEye() +
				camera.panSpeed*camera.getRight());
			camera.setCenter(camera.getCenter() +
				camera.panSpeed*camera.getRight());
	}
	else if (PRESS_SPACE) {
		camera.setEye(camera.getEye() +
			camera.panSpeed*camera.getUp());
		camera.setCenter(camera.getCenter() +
			camera.panSpeed*camera.getUp());
	}
	else if (PRESS_LSHIFT) {
		camera.setEye(camera.getEye() - 
			camera.panSpeed*camera.getUp());
		camera.setCenter(camera.getCenter() -
			camera.panSpeed*camera.getUp());
	}
	else if (PRESS_UP) {
		camera.rotateEye(-camera.getRight(), -0.2*camera.rollSpeed);
	}
	else if (PRESS_DOWN) {
		camera.rotateEye(-camera.getRight(), 0.2*camera.rollSpeed);
	}
	else if (PRESS_LEFT) {
		camera.rotateEye(-camera.getUp(), 0.5*camera.rollSpeed);
	}
	else if (PRESS_RIGHT) {
		camera.rotateEye(-camera.getUp(), -0.5*camera.rollSpeed);
	}
}

void MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
	lPos = cPos;
	cPos = glm::vec2(mouse_x, mouse_y);
	dV = lPos - cPos;
	dVl = glm::length(dV);
	if (!mousePressed)
		return;
	if (currentButton == GLFW_MOUSE_BUTTON_LEFT) {
		camera.rotateEye(camera.getUp(), camera.rotationSpeed * (dV.x / dVl)*.3);
		camera.rotateEye(camera.getRight(), camera.rotationSpeed * (dV.y / dVl)*.3);
	}
	else if (currentButton == GLFW_MOUSE_BUTTON_RIGHT) {
		camera.setEye(camera.getEye() -
			-0.1f*dV.y*camera.zoomSpeed*camera.getLook());
	}
	else if (PRESS_MIDDLE_BUTTON) {
		//std::cout << std::to_string(camera.getLook().z) << "\n";
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	mousePressed = (action == GLFW_PRESS);
	currentButton = button;
}
