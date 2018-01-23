#include "Sector.h"

Sector::Sector() : 
	temp(75.0), 
	texture(0), 
	position(vec3(0.0, 0.0, 0.0)) {
}

Sector::Sector(float x, float y, float z) : 
	temp(75.0), 
	texture(0), 
	position(vec3(x, y, z)) {
}

void Sector::drawSector(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs, vector<float> &temps) {
	// Local position vectors, used to build the cube with fewer operations
	vec3 relPP = vec3(size + position.x, size + position.y, size + position.z);
	vec3 relPN = vec3(-size + position.x, -size + position.y, -size + position.z);

	// Face 1 (Wall orthogonal to z-axis)
	vertices.push_back(vec4(relPN, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));

	// Face 2 (Wall opposite wall 1)
	vertices.push_back(vec4(relPN.x, relPN.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPP, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));

	// Face 3
	vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPP, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));

	// Face 4
	vertices.push_back(vec4(relPN, 1.0f));
	vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));

	// Face 5
	vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPP, 1.0f));
	vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));

	// Face 6
	vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPN, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
	vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));

	// Normals
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 12; j++) {
			normals.push_back(relN[i]);
		}
	}

	// Temperatures
	for (int i = 0; i < 36; i++) {
		temps.push_back(this->temp);
	}
}