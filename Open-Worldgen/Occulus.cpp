#include "Occulus.h"

Occulus::Occulus() :
	position(vec3(0.0f, 0.0f, 0.0f)),
	spacing(Chunk().size)
{
	size = spacing * 9.0;
	initMap(0.0f, 0.0f, 0.0f);
}

Occulus::Occulus(float x, float y, float z) :
	position(vec3(x, y, z)),
	spacing(Chunk().size * 2.0)
{
	size = spacing * 9.0;
	initMap(x, y, z);
}

Occulus::Occulus(vec3 pos) :
	position(vec3(pos.x, pos.y, pos.z)),
	spacing(Chunk().size * 2.0)
{
	size = spacing * 9.0;
	initMap(pos.x, pos.y, pos.z);
}

void Occulus::initMap(int x, int y, int z) {
	for (int i = -4; i < 5; i++) {
		for (int j = -4; j < 5; j++) {
			this->map.push_back(Chunk(x + j*spacing, 0.0f, z + i*spacing));
		}
	}
}

void Occulus::draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps, vector<uvec3> &faces) {
	int count = 0;
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			count = map[i * 9 + j].draw(vertices, normals, uvs, temps, faces, count, dMod + iMod[i] + jMod[j]);
		}
	}
}

