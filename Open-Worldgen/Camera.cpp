#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

Camera::Camera() : 
	cameraDistance(3.0f), 
	look(vec3(0.0f, 0.0f, -1.0f)), 
	up(vec3(0.0f, 1.0, 0.0f)), 
	center(vec3(0.0f, 0.0f, 0.0f)), 
	right(vec3(1.0, 0.0f, 0.0f)),
	eye(vec3(0.0f, 0.0f, cameraDistance)){}

Camera::Camera(float dist) : 
	cameraDistance(dist), 
	look(vec3(0.0f, 0.0f, -1.0f)), 
	up(vec3(0.0f, 1.0, 0.0f)), 
	center(vec3(0.0f, 0.0f, 0.0f)), 
	right(vec3(1.0, 0.0f, 0.0f)),
	eye(vec3(0.0f, 0.0f, cameraDistance)) {}

mat4 Camera::getViewMatrix() const
{
	vec3 Z = eye - center;
	Z = normalize(Z);
	vec3 Y = up;
	vec3 X = cross(Y, Z);
	Y = cross(Z, X);
	X = normalize(X);
	Y = normalize(Y);

	mat4 V = { X.x, X.y, X.z, dot(-X, eye),
		Y.x, Y.y, Y.z, dot(-Y, eye),
		Z.x, Z.y, Z.z, dot(-Z, eye),
		0.0f  , 0.0f  , 0.0f  , 1.0f };

	V = transpose(V);

	return V;
}

// Getters and setters beyond here (needed because setting some components updates multiple components)
vec3 Camera::getLook() { return look; }
vec3 Camera::getEye() { return eye; }
vec3 Camera::getUp() { return up; }
vec3 Camera::getRight() { return right; }
vec3 Camera::getCenter() { return center; }
void Camera::setLook(vec3 nLook) { look = nLook; }
void Camera::setEye(vec3 nEye) {
	cameraDistance = distance(nEye, center);
	eye = nEye;
}
void Camera::setUp(vec3 nUp) { up = nUp; }
void Camera::setRight(vec3 nRight) { right = nRight; }
void Camera::setCenter(vec3 nCenter) {
	cameraDistance = distance(nCenter, eye);
	center = nCenter;
}

void Camera::rotateEye(vec3 axis_of_rotation, float angle) {

	mat4x4 V = { right.x, up.x, look.x, eye.x,
		right.y, up.y, look.y, eye.y,
		right.z, up.z, look.z, eye.z,
		0.0f       , 0.0f    , 0.0f      , 1.0f };

	V = glm::rotate(V, angle, axis_of_rotation);
	right = vec3(V[0][0], V[1][0], V[2][0]);
	up = vec3(V[0][1], V[1][1], V[2][1]);
	look = vec3(V[0][2], V[1][2], V[2][2]);
	eye = vec3(V[0][3], V[1][3], V[2][3]);
	cameraDistance = distance(eye, center);
}