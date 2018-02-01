#include "Occulus.h"

Occulus::Occulus() :
	position(vec3(0.0f, 0.0f, 0.0f)),
	trimode(false),
	spacing(Chunk().size)
{
	size = spacing * O_DIM;
	initMap(0.0f, 0.0f, 0.0f, trimode);
}

Occulus::Occulus(float x, float y, float z) :
	position(vec3(x, y, z)),
	trimode(false),
	spacing(Chunk().size * 2.0)
{
	size = spacing * O_DIM;
	initMap(x, y, z, trimode);
}

Occulus::Occulus(vec3 pos) :
	position(vec3(pos.x, pos.y, pos.z)),
	trimode(false),
	spacing(Chunk().size * 2.0)
{
	size = spacing * O_DIM;
	initMap(pos.x, pos.y, pos.z, trimode);
}

void Occulus::update(vec3 pos) {
	map.clear();
	initMap(pos.x, pos.y, pos.z, trimode);
}

void Occulus::initMap(int x, int y, int z, int trimode) {
	for (int i = -O_MIN; i < O_MAX; i++) {
		for (int j = -O_MIN; j < O_MAX; j++) {
			this->map.push_back(Chunk(x + j*spacing - off[trimode] * j, 0.0, z + i*spacing - off[trimode] * i));
		}
	}
}

void Occulus::draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps, vector<uvec3> &faces) {
	if (trimode) {
		trimode = false;
		update(position);
	}
	int count = 0;
	for (int i = 0; i < O_DIM; i++) {
		for (int j = 0; j < O_DIM; j++) {
			count = map[i * O_DIM + j].draw(vertices, normals, uvs, temps, faces, count, dMod + iMod[i] + jMod[j]);
		}
	}
}

void Occulus::drawTrimesh(vector<vec4> &vertices, vector<vec4> &normals, vector<float> &temps,
						  vector<uvec3> &faces) {
	if (!trimode) {
		trimode = true;
		update(position);
	}
	int count = 0;
	for (int i = 0; i < O_DIM; i++) {
		for (int j = 0; j < O_DIM; j++) {
			count = map[i * O_DIM + j].drawTrimesh(vertices, normals, temps, faces, count);
		}
	}
}

