#include "Chunk.h"

Chunk::Chunk() :
	position(0.0f, 0.0f, 0.0f),
	spacing(Sector().size * 2.0)
{
	size = spacing * C_DIM;
	initMap(0.0f, 0.0f, 0.0f);
}

Chunk::Chunk(float x, float y, float z) : 
	position(x,y,z),
	spacing(Sector().size * 2.0)
{
	size = spacing * C_DIM;
	initMap(x, y, z);
}

Chunk::Chunk(vec3 pos) :
	position(pos.x, pos.y, pos.z),
	spacing(Sector().size * 2.0)
{
	size = spacing * C_DIM;
	initMap(pos.x, pos.y, pos.z);
}

int Chunk::draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps, vector<uvec3> &faces, int fNum, int draw) {
	int count = fNum;
	for (int i = 0; i < C_DIM; i++) {
		for (int j = 0; j < C_DIM; j++) {
			count = map[i*C_DIM + j].draw(vertices, normals, uvs, temps, faces, count, dMod + (draw & iMod[i]) + (draw & jMod[j]));
		}
	}
	return count;
}

int Chunk::drawTrimesh(vector<vec4> &vertices, vector<vec4> &normals,
					   vector<float> &temps, vector<uvec3> &faces, int fNum) {
	int limit = C_DIM - 1;
		for (int i = 0; i < limit; i++) {
			for (int j = 0; j < limit; j++) {
				vec4 p1 = vec4(map[i*C_DIM + j].position, 1.0);
				vec4 p2 = vec4(map[i*C_DIM + (j + 1)].position, 1.0);
				vec4 p3 = vec4(map[(i + 1)*C_DIM + j].position, 1.0);
				vec4 p4 = vec4(map[(i + 1)*C_DIM + (j + 1)].position, 1.0);
				float t1 = map[i*C_DIM + j].temp;
				float t2 = map[i*C_DIM + (j + 1)].temp;
				float t3 = map[(i + 1)*C_DIM + j].temp;
				float t4 = map[(i + 1)*C_DIM + (j + 1)].temp;
				vec4 n1 = calcNormal(p1, p3, p2);
				vec4 n2 = calcNormal(p2, p3, p4);
				//cout << n2.x << " " << n2.y << " " << n2.z << " " << n2.w << endl;

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
	return fNum;
}

vec4 Chunk::calcNormal(vec3 p1, vec3 p2, vec3 p3) {
	vec3 U = normalize(p2 - p1);
	vec3 V = normalize(p3 - p1);

	vec3 normal = vec3(U.y * V.z - U.z * V.y,
					   U.z * V.x - U.x * V.z,
					   U.x * V.y - U.y * V.x);

	return vec4(normalize(normal), 1.0f);
}

void Chunk::initMap(float x, float y, float z) {
	for (int i = C_MIN; i < C_MAX; i++) {
		for (int j = C_MIN; j < C_MAX; j++) {
			map.push_back(Sector(x + j*spacing, y, z + i*spacing));
		}
	}
}