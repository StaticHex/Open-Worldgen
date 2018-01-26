#include "Chunk.h"

Chunk::Chunk() :
	position(0.0f, 0.0f, 0.0f),
	spacing(Sector().size * 2.0)
{
	size = spacing * 16.0;
	for (int i = -8; i < 8; i++) {
		for (int j = -8; j < 8; j++) {
			map.push_back(Sector(j*spacing, 0.0f, i*spacing));
		}
	}
}

Chunk::Chunk(float x, float y, float z) : 
	position(x,y,z),
	spacing(Sector().size * 2.0)
{
	size = spacing * 16.0;
	for (int i = -8; i < 8; i++) {
		for (int j = -8; j < 8; j++) {
			map.push_back(Sector(this->position.x + j*spacing, 0.0f ,this->position.z + i*spacing));
		}
	}
}

Chunk::Chunk(vec3 pos) :
	position(pos.x, pos.y, pos.z),
	spacing(Sector().size * 2.0)
{
	size = spacing * 16.0;
	for (int i = -8; i < 8; i++) {
		for (int j = -8; j < 8; j++) {
			map.push_back(Sector(this->position.x + j*spacing, 0.0f, this->position.z + i*spacing));
		}
	}
}

int Chunk::draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps, vector<uvec3> &faces, int fNum, int draw) {
	int count = fNum;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			count = map[i*16 + j].draw(vertices, normals, uvs, temps, faces, count, dMod + (draw & iMod[i]) + (draw & jMod[j]));
		}
	}
	return count;
}