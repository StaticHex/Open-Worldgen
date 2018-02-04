#pragma once
#define C_DIM 16
#define O_NUM 3
#define O_DIM C_DIM * O_NUM
#define O_MIN -(O_DIM / 2)
#define O_MAX (O_DIM / 2)
#include "Sector.h"

class Occulus {
public:
	vec3 position;
	float size;
	int xCount;
	int zCount;
	vector<Sector> map;
	Occulus();
	Occulus(float x, float y, float z);
	Occulus(vec3 pos);
	void draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps, vector<uvec3> &faces);
	void update(vec3 pos, vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps);
private:
	vec3 cPos;
	vec3 dPos;
	float spacing;
	void initMap(int x, int y, int z);
	vec4 calcNormal(vec3 p1, vec3 p2, vec3 p3);
	void draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps);
};