#pragma once
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib, "glu32.lib")
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <Windows.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <FL/fl.h>
#include <FL/gl.h>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/GLU.h>
using glm::vec3;
using glm::vec2;

class GlWindow : public Fl_Gl_Window {
	void draw();
	int	handle(int);
public:
	int			windowWidth;	// Holds the width of the window
	int			windowHeight;	// Holds the height of the window
	vec3		eye;			// Vector which holds the location of the eye
	vec3		center;		// Vector which holds the location of the center
	vec3		up;			// Vector which holds the direction of up
	vec3		look;			// Vector which holds the direction we're looking (orthogonal to up and right)
	vec3		right;			// Vector which holds the direction of right (orthogonal to up and look)

	GlWindow(int X, int Y, int W, int H, const char* L) : Fl_Gl_Window(X, Y, W, H, L) {
		// Store the window's height and width (used to keep mouse controls in bounds)
		windowWidth = W;
		windowHeight = H;

		// Set the rotation speed, pan speed, and zoom speed
		rotSpeed = 0.035f;
		panSpeed = 0.1f;
		zoomSpeed = 0.1f;

		// set initial values for the last position and velocity of the cursor
		lPos = vec2(0.0f, 0.0f);
		dV = vec2(0.0f, 0.0f);

		// Set initial positions for camera vectors
		eye = vec3(0.0f, 0.0f, 3.0f);
		center = vec3(0, 0, 0);
		up = vec3(0.0f, 1.0f, 0.0f);
		look = vec3(0.0f, 0.0f, -1.0f);
		right = vec3(1.0, 0.0f, 0.0f);

		// Set sector size
		secSize = 1.0;

		// set up OpenGL to render
		mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);

		// Set up and attach shaders here
	};
	void drawSector(float x, float y, float z);
	void resetView();
private:
	// shader progams
	const char* vertexShaderSrc = "";
	const char* fragmentShaderSrc = "";

	vec2		lPos;		// last Position, holds the cursor's previous x and y position
	vec2		dV;			// delta Vector, stores the change in the mouse cursor's position between this update and the last
	float		secSize;	// The size of each sector, 1m by default
	float		rotSpeed;	// Constant used to help set the speed of rotation for the die (helps smooth the rotation out)
	float		panSpeed;	// Constant used to help set the speed of panning for the die (helps to smooth movement out)
	float		zoomSpeed;	// Constant used to help adjust the zoom for the image (helps to smooth movement out)
	void rotateEye(vec3 AOR, float angle);
	void setLook(vec3 ePos, vec3 cPos, vec3 uPos);
};
