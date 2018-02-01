#pragma once
#include "Sector.h"

#define C_DIM 16
#define C_MIN -C_DIM / 2
#define C_MAX C_DIM / 2

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
	int drawTrimesh(vector<vec4> &vertices, vector<vec4> &normals,
		vector<float> &temps, vector<uvec3> &faces, int fNum);
private:
	float spacing; // used to space sectors apart to form a uniform grid
	// used to pass which faces of each sector to draw, arrays used to eliminate if statements
	const int dMod = 48;
	const int iMod[C_DIM] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 };
	const int jMod[C_DIM] = { 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4 };
	vec4 calcNormal(vec3 p1, vec3 p2, vec3 p3);
	void initMap(float x, float y, float z);
};