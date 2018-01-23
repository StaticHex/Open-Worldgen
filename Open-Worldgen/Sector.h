#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
using glm::vec4;
using glm::vec3;
using glm::vec2;
using std::vector;
using std::cout;
using std::endl;

class Sector {
public:
	float temp;
	int texture;
	vec3 position;

	Sector();
	Sector(float x, float y, float z);
	void drawSector(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs, vector<float> &temps);
private:
	const float size = 1.0;
	const vec4 relN[3] = { vec4(0.0f, 0.0f, 1.0f, 0.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f) };
};