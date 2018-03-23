#pragma once
#define C_DIM 16
#define O_NUM 12
#define O_DIM C_DIM * O_NUM
#define O_MIN -(O_DIM / 2)
#define O_MAX (O_DIM / 2)
#define UVX_MIN 0.0
#define UVX_MAX 0.24
#define UVY_MIN 0.0
#define UVY_MAX 0.50

#include "Sector.h"
#include "OpenSimplex.h"
#include <map>
#include <iterator>

using glm::cross;

class Occulus {
public:
	vec3 position;
	float size;
	float seaLevel;
	vector<Sector> map;
	Occulus();
	Occulus(float x, float y, float z);
	Occulus(vec3 pos);
	void draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
		vector<float> &temps, vector<float> &heights, vector<uvec3> &faces);
	void drawWater(vector<vec4> &vertices, vector<vec2> &uvs, vector<uvec3>&faces);
	void update(vec3 pos, vector<float> &heights, vector<vec4> &normals, vector<float> &temps);
private:
	float spacing;
	void initMap();
	void updateMap();
	void mapNoise(int index);
	vector<vec4> nIndex; // TODO: Delete this once indexed vertices are implemented
	std::map<int, int> indexMap;
	struct osn_context *ctx;
	vec4 calcNormal(vec3 p1, vec3 p2, vec3 p3);
	void draw(vector<vec4> &normals, vector<float> &temps, vector<float> &heights);
};