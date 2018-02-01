#pragma once
#include "Chunk.h"
#define O_DIM 3
#define O_MIN O_DIM / 2
#define O_MAX O_DIM / 2 + 1

class Occulus {
public:
	vec3 position;
	float size;
	vector<Chunk> map;
	Occulus();
	Occulus(float x, float y, float z);
	Occulus(vec3 pos);
	void draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps, vector<uvec3> &faces);
	void drawTrimesh(vector<vec4> &vertices, vector<vec4> &normals, 
		vector<float> &temps, vector<uvec3> &faces);
	void update(vec3 pos);
private:
	float spacing;
	bool trimode = false;
	const int dMod = 48;
	const int iMod[O_DIM] = { 1, 0, 2 };
	const int jMod[O_DIM] = { 8, 0, 4 };
	const float off[2] = { 0.0, Sector().size * 2 };
	void initMap(int x, int y, int z, int trimode);
};