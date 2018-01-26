#pragma once
#include "Sector.h"

class Chunk {
public:
	vec3 position;
	vector<Sector> map;
	float size;
	Chunk();
	Chunk(float x, float y, float z);
	Chunk(vec3 pos);
	int draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		      vector<float> &temps, vector<uvec3> &faces, int fNum, int draw);
private:
	float spacing; // used to space sectors apart to form a uniform grid
	// used to pass which faces of each sector to draw, arrays used to eliminate if statements
	const int dMod = 48;
	const int iMod[16] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 };
	const int jMod[16] = { 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4 };
};