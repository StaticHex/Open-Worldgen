#include "Occulus.h"
using glm::clamp;

Occulus::Occulus() :
	spacing(Sector().size * 2.0)
{
	position = vec3(0.0f, 0.0f, 0.0f);
	size = spacing * O_DIM * C_DIM;
	xCount = O_DIM - 1;
	zCount = O_DIM - 1;
	initMap(0.0f, 0.0f, 0.0f);
}

Occulus::Occulus(float x, float y, float z) :
	spacing(Sector().size * 2.0)
{
	position = vec3(x, y, z);
	size = spacing * O_DIM * C_DIM;
	xCount = O_DIM - 1;
	zCount = O_DIM - 1;
	initMap(x, y, z);
}

Occulus::Occulus(vec3 pos) :
	position(vec3(pos.x, pos.y, pos.z)),
	spacing(Sector().size * 2.0)
{
	size = spacing * O_DIM;
	xCount = O_DIM - 1;
	zCount = O_DIM - 1;
	initMap(pos.x, pos.y, pos.z);
}

void Occulus::initMap(int x, int y, int z) {
	if (!map.size()) {
		for (int i = O_MIN; i < O_MAX; i++) {
			for (int j = O_MIN; j < O_MAX; j++) {
				this->map.push_back(Sector(x + j*spacing, 0.0f, z + i*spacing));
			}
		}
	}
	else {
		for (int i = O_MIN; i < O_MAX; i++) {
			for (int j = O_MIN; j < O_MAX; j++) {
				this->map[(i+O_MAX)*O_DIM + (j+O_MAX)].position = vec3(x + j*spacing, 0.0f, z + i*spacing);
			}
		}
	}
}

void Occulus::update(vec3 pos, vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps) {
	initMap(pos.x, pos.y, pos.z);
	// set temperature
	for (int i = 0; i < O_DIM; i++) {
		for (int j = 0; j < O_DIM; j++) {
			if ((i + j) % 2 == 0)
				map[i * O_DIM + j].temp = 50;
			else
				map[i * O_DIM + j].temp = 75;
		}
	}
	draw(vertices, normals, uvs, temps);
}

void Occulus::draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps, vector<uvec3> &faces) {
	int fNum = 0;
	int limit = O_DIM - 1;
	for (int i = 0; i < limit; i++) {
		for (int j = 0; j < limit; j++) {
			vec4 p1 = vec4(map[i*O_DIM + j].position, 1.0);
			vec4 p2 = vec4(map[i*O_DIM + (j + 1)].position, 1.0);
			vec4 p3 = vec4(map[(i + 1)*O_DIM + j].position, 1.0);
			vec4 p4 = vec4(map[(i + 1)*O_DIM + (j + 1)].position, 1.0);
			float t1 = map[i*O_DIM + j].temp;
			float t2 = map[i*O_DIM + (j + 1)].temp;
			float t3 = map[(i + 1)*O_DIM + j].temp;
			float t4 = map[(i + 1)*O_DIM + (j + 1)].temp;
			vec4 n1 = calcNormal(p1, p3, p2);
			vec4 n2 = calcNormal(p2, p3, p4);

			vertices.push_back(p1);
			temps.push_back(t1);
			normals.push_back(n1);
			vertices.push_back(p2);
			temps.push_back(t2);
			normals.push_back(n1);
			vertices.push_back(p3);
			temps.push_back(t3);
			normals.push_back(n1);
			faces.push_back(vec3(fNum, fNum + 1, fNum + 2));
			fNum += 3;

			vertices.push_back(p3);
			temps.push_back(t3);
			normals.push_back(n2);
			vertices.push_back(p4);
			temps.push_back(t4);
			normals.push_back(n2);
			vertices.push_back(p2);
			temps.push_back(t2);
			normals.push_back(n2);
			faces.push_back(vec3(fNum, fNum + 1, fNum + 2));
			fNum += 3;
		}
	}
}

void Occulus::draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps) {
	int limit = O_DIM - 1;
	for (int i = 0; i < limit; i++) {
		for (int j = 0; j < limit; j++) {
			vec4 p1 = vec4(map[i*O_DIM + j].position, 1.0);
			vec4 p2 = vec4(map[i*O_DIM + (j + 1)].position, 1.0);
			vec4 p3 = vec4(map[(i + 1)*O_DIM + j].position, 1.0);
			vec4 p4 = vec4(map[(i + 1)*O_DIM + (j + 1)].position, 1.0);
			float t1 = map[i*O_DIM + j].temp;
			float t2 = map[i*O_DIM + (j + 1)].temp;
			float t3 = map[(i + 1)*O_DIM + j].temp;
			float t4 = map[(i + 1)*O_DIM + (j + 1)].temp;
			vec4 n1 = calcNormal(p1, p3, p2);
			vec4 n2 = calcNormal(p2, p3, p4);

			vertices[6 * (i*limit + j)] = p1;
			temps[6 * (i*limit + j)]= t1;
			normals[6 * (i*limit + j)] = n1;
			vertices[6 * (i*limit + j) + 1] = p2;
			temps[6 * (i*limit + j) + 1] = t2;
			normals[6 * (i*limit + j) + 1] = n1;
			vertices[6 * (i*limit + j) + 2] = p3;
			temps[6 * (i*limit + j) + 2] = t3;
			normals[6 * (i*limit + j) + 2] = n1;

			vertices[6 * (i*limit + j) + 3] = p3;
			temps[6 * (i*limit + j) + 3] = t3;
			normals[6 * (i*limit + j) + 3] = n2;
			vertices[6 * (i*limit + j) + 4] = p4;
			temps[6 * (i*limit + j) + 4] = t4;
			normals[6 * (i*limit + j) + 4] = n2;
			vertices[6 * (i*limit + j) + 5] = p2;
			temps[6 * (i*limit + j) + 5] = t2;
			normals[6 * (i*limit + j) + 5] = n2;
		}
	}
}

vec4 Occulus::calcNormal(vec3 p1, vec3 p2, vec3 p3) {
	vec3 U = normalize(p2 - p1);
	vec3 V = normalize(p3 - p1);

	vec3 normal = vec3(U.y * V.z - U.z * V.y,
		U.z * V.x - U.x * V.z,
		U.x * V.y - U.y * V.x);

	return vec4(normalize(normal), 1.0f);
}

