#pragma once
#define C_DIM 16
#define O_NUM 20
#define O_DIM C_DIM * O_NUM
#define O_MIN -(O_DIM / 2)
#define O_MAX (O_DIM / 2)
#define UVX_MIN 0.0
#define UVX_MAX 0.24
#define UVY_MIN 0.0
#define UVY_MAX 0.50

#include "OpenSimplex.h"
#include "Sector.h"
#include <map>
#include <vector>
#include<glm/glm.hpp>
#include <iterator>
#include <thread>
#include <mutex>
#include <mutex>
#include <iostream>

using glm::cross;
using std::vector;
using glm::vec3;
using glm::vec4;
using glm::vec2;
using glm::uvec3;
using std::tuple;
using std::get;

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
	void update(vec3 pos, vector<float> &heights, vector<vec4> &normals, vector<float> &temps, vector<uvec3> &faces);
private:
	float spacing;
	void initMap();
	void updateMap();
	tuple<float, float> mapNoise(vec3 pos);
	vector<vec4> nIndex; // TODO: Delete this once indexed vertices are implemented
	vector<int> indexes;
	struct osn_context *ctx;
	vec3 lPosition;
	vec4 calcNormal(vec3 p1, vec3 p2, vec3 p3);
	void draw(vector<vec4> &normals, vector<float> &temps, vector<float> &heights, vector<uvec3> &faces);
	int calcFlags();
	void runGenRow();
	void runGenCol();
	void genRow(int flags, vector<tuple<float, float>> &row);
	void genCol(int flags, vector<tuple<float, float>> &col);
};