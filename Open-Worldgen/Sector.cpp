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

Sector::Sector(vec3 pos) :
	temp(75.0),
	texture(0),
	position(vec3(pos.x, pos.y, pos.z)) {
}

// Draw Sector Function
// @param
// - vertices: Location of the place to store the vertices for the sector
// - normals: Location of the place to store the normals for the sector
// - uvs: Location of the place to store the uv values for the sector
// - temps: Location of the place to store the temperature values for the sector
// - faces: Location of the place to store the faces for the sector
// - fNum: Used to store and pass the number of faces drawn for all sectors between draw
//         functions. Allows multiple sectors to be drawn at once
// - draw: A bit vector used to determine which faces of each sector to draw. Helps to 
//         eliminate internal geometry without a bunch of complex checks
// @description
// - Sets up all information needed to draw the bounding box of the sector.
int Sector::draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs, 
	vector<float> &temps, vector<uvec3> &faces, int fNum, int draw) {
	// Local position vectors, used to build the cube with fewer operations
	vec3 relPP = vec3(size + position.x, size + position.y, size + position.z);
	vec3 relPN = vec3(-size + position.x, -size + position.y, -size + position.z);
	int nDrawn = (draw & 1) + ((draw & 2) >> 1) + ((draw & 4) >> 2);
		nDrawn += ((draw & 8) >> 3) + ((draw & 16) >> 4) + ((draw & 32) >> 5);

	// Face 1 (back)
	if (draw & 1) {
		vertices.push_back(vec4(relPN, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));
		for (int i = 0; i < 6; i++) {
			normals.push_back(relN[0]);
		}
	}

	if (draw & 2) {
		// Face 2 (front)
		vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPP, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
		for (int i = 0; i < 6; i++) {
			normals.push_back(relN[0]);
		}
	}

	if (draw & 4) {
		// Face 3 (right)
		vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPP, 1.0f));
		vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));
		for (int i = 0; i < 6; i++) {
			normals.push_back(relN[1]);
		}
	}

	if (draw & 8) {
		// Face 4 (left)
		vertices.push_back(vec4(relPN, 1.0f));
		vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));
		for (int i = 0; i < 6; i++) {
			normals.push_back(relN[1]);
		}
	}

	if (draw & 16) {

		// Face 5 (top)
		vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPP, 1.0f));
		vertices.push_back(vec4(relPN.x, relPP.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPP.y, relPN.z, 1.0f));
		for (int i = 0; i < 6; i++) {
			normals.push_back(relN[2]);
		}
	}

	if (draw & 32) {
		// Face 6 (bottom)
		vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPN, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPN.x, relPN.y, relPP.z, 1.0f));
		vertices.push_back(vec4(relPP.x, relPN.y, relPN.z, 1.0f));
		for (int i = 0; i < 6; i++) {
			normals.push_back(relN[2]);
		}
	}

	// Temperatures
	int stop = 6 * nDrawn;
	for (int i = 0; i < stop; i++) {
		temps.push_back(this->temp);
	}

	int start = fNum;
	int end = start + 6*nDrawn;
	for (int i = start; i < end; i += 3) {
		faces.push_back(uvec3(i, i + 1, i + 2));
	}
	return end;
}