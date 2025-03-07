#pragma once
#include "settings.h"
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
#include <thread>
#include <stdlib.h> /* atoi */

// OpenGL library includes
#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>

// Project includes
#include "Occulus.h"
#include "Camera.h"
#include "debuggl.h"

// GUI Libraries
#include <iostream>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <fl/Fl_Widget.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Value_Slider.H>

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::uvec3;
using glm::perspective;
using glm::radians;
using std::cout;
using std::endl;
using std::numeric_limits;

// Init settings
double seaLevel = 1.0;

// Elevation settings
double height1a = 1.0;
double height1b = 10.0;
double height1c = 10.0;
double height2a = 0.5;
double height2b = 20.0;
double height2c = 20.0;
double height3a = 0.25;
double height3b = 50.0;
double height3c = 30.0;

// Raise elevation by power
double heightPow = 2.33334;

// sea level settings
double slHeighta = 0.25;
double slHeightb = 5.0;
double slHeightc = 5.0;

// multiplier
double maxH = 20.001;

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
int centerX = 0;
int centerY = 0;
bool isGlobal = true;
int currentButton;
bool mousePressed;
float aspect = 0.0f;

// Grid variable
bool setRefresh = false;

// UI Variables
int fps = 0;
bool running = true; // used for threading UI updates
Fl_Window *mainWindow = new Fl_Window(120, 400, "Settings"); // Main window for settings toolbar
Fl_Box *box = new Fl_Box(10, 40, 100, 40, "###"); // Box which holds the FPS counter

double slWidth = 400.0;
double slHeight = 80.0;
// Window which holds the settings for adjusting the sea level for the map
Fl_Window *seaLevelPanel = new Fl_Window(Fl::w() / 2.0 - slWidth / 2.0, Fl::h() / 2.0 - slHeight / 2.0, slWidth, slHeight, "Sea Level");
Fl_Box *lblSeaLevel = new Fl_Box(0, 0, seaLevelPanel->w(), 30, "Change Sea Level"); // Header for sea level window
Fl_Button *btnSeaBack = new Fl_Button(10, lblSeaLevel->h() / 2.0 - (20/2.0), 70, 20, "<< Back");
Fl_Value_Slider *seaSlider = new Fl_Value_Slider(10, lblSeaLevel->h() + 10, seaLevelPanel->w() - 20, seaLevelPanel->h() - lblSeaLevel->h() - 20, "");

// Window which holds the settings for adjusting the noise constants for the map
double anWidth = 400.0;
double anHeight = 290.0;
Fl_Window *adjustNoise = new Fl_Window(Fl::w() / 2.0 - anWidth / 4.0, Fl::h() / 2.0 - anHeight / 4.0, anWidth, anHeight, "Adjust Noise");
Fl_Box *lblNoiseGen = new Fl_Box(0, 0, adjustNoise->w(), 30, "Change Noise Settings");
Fl_Button *btnNoiseBack = new Fl_Button(10, lblNoiseGen->h() / 2.0 - (20 / 2.0), 70, 20, "<< Back");

// Elevation sliders
const double nSliderW = (adjustNoise->w() - 40) / 3.0; // calculate width of elevation sliders dynamically
const double nSliderMW = adjustNoise->w() - (nSliderW) * 2.0 - 40;
const double nSliderX1 = 10;

const double nSliderH1 = lblNoiseGen->y() + lblNoiseGen->h() + 30;
Fl_Value_Slider *noiseElevation1a = new Fl_Value_Slider(nSliderX1, nSliderH1, nSliderW, 20, "Elevation Settings");

const double nSliderX2 = noiseElevation1a->x() + noiseElevation1a->w() + 10;
Fl_Value_Slider *noiseElevation1b = new Fl_Value_Slider(nSliderX2,nSliderH1,  nSliderMW, 20);

const double nSliderX3 = noiseElevation1b->x() + noiseElevation1b->w() + 10;
Fl_Value_Slider *noiseElevation1c = new Fl_Value_Slider(nSliderX3, nSliderH1, nSliderW, 20);

const double nSliderH2 = noiseElevation1a->y() + noiseElevation1a->h() + 5;
Fl_Value_Slider *noiseElevation2a = new Fl_Value_Slider(nSliderX1, nSliderH2, nSliderW, 20);
Fl_Value_Slider *noiseElevation2b = new Fl_Value_Slider(nSliderX2, nSliderH2, nSliderMW, 20);
Fl_Value_Slider *noiseElevation2c = new Fl_Value_Slider(nSliderX3, nSliderH2, nSliderW, 20);

const double nSliderH3 = noiseElevation2a->y() + noiseElevation2a->h() + 5;
Fl_Value_Slider *noiseElevation3a = new Fl_Value_Slider(nSliderX1, nSliderH3, nSliderW, 20);
Fl_Value_Slider *noiseElevation3b = new Fl_Value_Slider(nSliderX2, nSliderH3, nSliderMW, 20);
Fl_Value_Slider *noiseElevation3c = new Fl_Value_Slider(nSliderX3, nSliderH3, nSliderW, 20);

// Elevation power constant slider
const double npSliderH = noiseElevation3a->y() + noiseElevation3a->h() + 30;
Fl_Value_Slider *sldEPower = new Fl_Value_Slider(nSliderX1, npSliderH, adjustNoise->w() - 20 , 20, "Elevation Power Constant");

// sea level sliders
const double nsSliderH = sldEPower->y() + sldEPower->h() + 30;
Fl_Value_Slider *sldSeaNoiseA = new Fl_Value_Slider(nSliderX1, nsSliderH, nSliderW, 20, "Sea Bed Settings");
Fl_Value_Slider *sldSeaNoiseB = new Fl_Value_Slider(nSliderX2, nsSliderH, nSliderMW, 20);
Fl_Value_Slider *sldSeaNoiseC = new Fl_Value_Slider(nSliderX3, nsSliderH, nSliderW, 20);

// Height multiplier slider
const double hmSliderH = sldSeaNoiseA->y() + sldSeaNoiseA->h() + 30;
Fl_Value_Slider *sldHMult = new Fl_Value_Slider(nSliderX1, hmSliderH, adjustNoise->w() - 20, 20, "Height Multiplier");

Fl_Box *lbl_fps = new Fl_Box(10, 0, 80, 40, "FPS"); // Label for the FPS counter

// images for the settings window buttons
Fl_PNG_Image *imgSeaLvl = new Fl_PNG_Image("./assets/sealevel.png");
Fl_PNG_Image *imgSeaLvlInactive = new Fl_PNG_Image("./assets/sealevel-inactive.png");
Fl_PNG_Image *imgNoiseP = new Fl_PNG_Image("./assets/aterrain.png");
Fl_PNG_Image *imgNoisePInactive = new Fl_PNG_Image("./assets/aterrain.-inactive.png");

Fl_Button *btnSeaLvl = new Fl_Button(10, 90, 45, 30, ""); // button which brings up the sea level window
Fl_Button *btnNoiseP = new Fl_Button(65, 90, 45, 30, ""); // button which brings up the noise settings window

// Mouse position vectors
vec2 cPos = vec2(0, 0);
vec2 lPos = vec2(0, 0);
vec2 dV = vec2(0, 0);
float dVl = glm::length(dV);

// Terrain Shader Variables
vector<vec4> tVertices;
vector<float> tTemps;
vector<float> tHeights;
vector<vec4> tNormals;
vector<uvec3> tFaces;
vector<vec2> tUv;

// Water Shader Variables
vector<vec4> wVertices;
vector<uvec3> wFaces;
vector<vec2> wUV;

// Create the camera
Camera camera = Camera(3.0);

// upper and lower bounds
vec4 min_bounds = vec4(-(numeric_limits<float>::max)());
vec4 max_bounds = vec4((numeric_limits<float>::max)());

// Universal shader constants
vec4 lightPos = vec4(0.0f, 10000.0f, 0.0f, 1.0f);
const float ambConstant = 0.10f;

// Terrain Shader constants
const vec4 tAmbient = vec4(1.0f, 1.0f, 0.875f, 1.0f);
const vec4 tSpecular = vec4(0.01f, 0.01f, 0.01f, 1.0f);
const vec4 tSceneCol = vec4(0.0f, 0.8f, 1.0f, 0.0f);
const float tFogDist = 110.0;
const float tShininess = 0.01f;

// Water Shader constants
const vec4 wAmbient = vec4(1.0f, 1.0f, 0.875f, 1.0f);
const vec4 wSpecular = vec4(0.5f, 1.0f, 1.0f, 1.0f);
const float wShininess = 10.0f;
int wTime = 0;

// VBO and VAO descriptors
enum { kVertexBuffer, kNormalBuffer, kTempBuffer, kHeightBuffer, kUVBuffer, kIndexBuffer, kNumVbos };

// VAOs
enum { kGeometryVao, kWaterVao, kNumVaos };

//Variables to hold VAO and VBO descriptors
GLuint gArrayObjects[kNumVaos]; // Holds VAO descriptors
GLuint gBufferObjects[kNumVaos][kNumVbos]; // Holds VBO descriptors

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

void showPanelCallback(Fl_Widget* widget, void* target) {
	Fl_Window *current = (Fl_Window*)target;

	if (current->visible()) {
		current->hide();
	}
	else {
		current->show();
	}
}

void hidePanelCallback(Fl_Widget* widget, void* target) {
	Fl_Window *current = (Fl_Window*)target;
	current->hide();
}

void sliderCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	seaLevel = sld->value();
}

void sliderEH1ACallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height1a = sld->value();
	setRefresh = true;
}

void sliderEH1BCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height1b = sld->value();
	setRefresh = true;
}

void sliderEH1CCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height1c = sld->value();
	setRefresh = true;
}

void sliderEH2ACallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height2a = sld->value();
	setRefresh = true;
}

void sliderEH2BCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height2b = sld->value();
	setRefresh = true;
}

void sliderEH2CCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height2c = sld->value();
	setRefresh = true;
}

void sliderEH3ACallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height3a = sld->value();
	setRefresh = true;
}

void sliderEH3BCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height3b = sld->value();
	setRefresh = true;
}

void sliderEH3CCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	height3c = sld->value();
	setRefresh = true;
}

void sldEPowerCCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	heightPow = sld->value();
	setRefresh = true;
}

void sldSeaNoiseACallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	slHeighta = sld->value();
	setRefresh = true;
}

void sldSeaNoiseBCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	slHeightb = sld->value();
	setRefresh = true;
}

void sldSeaNoiseCCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	slHeightc = sld->value();
	setRefresh = true;
}

void sldHMultCallback(Fl_Widget* widget, void* target) {
	Fl_Value_Slider* sld = (Fl_Value_Slider*)target;
	maxH = sld->value();
	setRefresh = true;
}

