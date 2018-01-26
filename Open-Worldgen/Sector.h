#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
using glm::vec4;
using glm::vec3;
using glm::uvec3;
using glm::vec2;
using std::vector;
using std::cout;
using std::endl;

class Sector {
public:
	float temp;
	int texture;
	const float size = 0.25f;
	vec3 position;

	Sector();
	Sector(float x, float y, float z);
	Sector(vec3 pos);
	int draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps, vector<uvec3> &faces, int fNum, int draw);
private:
	const vec4 relN[3] = { vec4(0.0f, 0.0f, 1.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f) };
};