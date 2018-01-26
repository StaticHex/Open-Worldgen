#pragma once
#include "Chunk.h"

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
private:
	float spacing;
	const int dMod = 48;
	const int iMod[16] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 };
	const int jMod[16] = { 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4 };
	void initMap(int x, int y, int z);
};