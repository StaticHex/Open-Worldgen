#pragma once
#define C_DIM 16
#define O_NUM 20
#define O_DIM C_DIM * O_NUM
#define O_MIN -(O_DIM / 2)
#define O_MAX (O_DIM / 2)
#include "Sector.h"
#include "OpenSimplex.h"
using glm::cross;

class Occulus {
public:
	vec3 position;
	float size;
	vector<Sector> map;
	Occulus();
	Occulus(float x, float y, float z);
	Occulus(vec3 pos);
	void draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps, vector<uvec3> &faces);
	void update(vec3 pos, vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps);
private:
	float spacing;
	void initMap();
	void updateMap();
	void mapNoise(int index);
	vector<vec4> nIndex;
	struct osn_context *ctx;
	vec4 calcNormal(vec3 p1, vec3 p2, vec3 p3);
	void smoothShading(vector<vec4> &normals);
	void draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps);
};