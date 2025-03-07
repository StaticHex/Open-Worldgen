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
	eye(vec3(0.0f, 0.0f, cameraDistance)){
	orientation = mat3(right, up, look);
}

Camera::Camera(float dist) : 
	cameraDistance(dist), 
	look(vec3(0.0f, 0.0f, -1.0f)), 
	up(vec3(0.0f, 1.0, 0.0f)), 
	center(vec3(0.0f, 0.0f, 0.0f)), 
	right(vec3(1.0, 0.0f, 0.0f)),
	eye(vec3(0.0f, 0.0f, cameraDistance)) {
	orientation = mat3(right, up, look);
}

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

	//return V;
	return glm::lookAt(eye, center, up);
}

// Getters and setters beyond here (needed because setting some components updates multiple components)
vec3 Camera::getLook() { return look; }
vec3 Camera::getEye() { return eye; }
vec3 Camera::getUp() { return up; }
vec3 Camera::getRight() { return right; }
vec3 Camera::getCenter() { return center; }
void Camera::setLook(vec3 nLook) { look = nLook; }
void Camera::setEye(vec3 nEye) {
	//float factor = Sector().size * 2;
	//nEye.x = roundf(nEye.x / factor) * factor;
	//nEye.z = roundf(nEye.z / factor) * factor;
	cameraDistance = distance(nEye, center);
	eye = nEye;
}
void Camera::setUp(vec3 nUp) { up = nUp; }
void Camera::setRight(vec3 nRight) { right = nRight; }
void Camera::setCenter(vec3 nCenter) {
	//float factor = Sector().size * 2;
	//nCenter.x = roundf(nCenter.x / factor) * factor;
	//nCenter.z = roundf(nCenter.z / factor) * factor;
	cameraDistance = distance(nCenter, eye);
	center = nCenter;
}

void Camera::rotateEye(vec3 axis_of_rotation, float angle) {
	orientation = mat3(rotate(-angle, axis_of_rotation) * mat4(orientation));
	right = column(orientation, 0);
	look = column(orientation, 1);
	look = column(orientation, 2);
	center = eye + cameraDistance * look;
	cameraDistance = distance(eye, center);
}