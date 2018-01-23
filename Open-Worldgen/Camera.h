#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
using glm::mat4;
using glm::vec3;
using glm::distance;
using glm::normalize;
using glm::cross;
using glm::dot;
using glm::transpose;
using glm::mat4x4;

class Camera {
public:
	const float panSpeed = 0.1f;
	const float rollSpeed = 0.1f;
	const float rotationSpeed = 0.05f;
	const float zoomSpeed = 0.1f;
	float cameraDistance;
	Camera();
	Camera(float dist);
	mat4 getViewMatrix() const;
	void rotateEye(vec3 axis_of_rotation, float angle);
	void setEye(vec3 nEye);
	void setLook(vec3 nLook);
	void setUp(vec3 nUp);
	void setCenter(vec3 nCenter);
	void setRight(vec3 nRight);
	vec3 getEye();
	vec3 getLook();
	vec3 getUp();
	vec3 getCenter();
	vec3 getRight();
private:
	vec3 eye;
	vec3 look;
	vec3 up;
	vec3 center;
	vec3 right;
	// Note: you may need additional member variables
};

#endif