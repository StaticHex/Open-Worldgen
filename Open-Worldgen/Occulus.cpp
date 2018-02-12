#include "Occulus.h"
using glm::clamp;

Occulus::Occulus() :
	spacing(Sector().size * 2.0)
{
	position = vec3(0.0f, 0.0f, 0.0f);
	size = spacing * O_DIM * C_DIM;
	open_simplex_noise(77374, &ctx);
	initMap();
}

Occulus::Occulus(float x, float y, float z) :
	spacing(Sector().size * 2.0)
{
	position = vec3(x, y, z);
	size = spacing * O_DIM * C_DIM;
	open_simplex_noise(77374, &ctx);
	initMap();
}

Occulus::Occulus(vec3 pos) :
	position(vec3(pos.x, pos.y, pos.z)),
	spacing(Sector().size * 2.0)
{
	size = spacing * O_DIM;
	open_simplex_noise(77374, &ctx);
	initMap();
}

void Occulus::mapNoise(int index) {
	vec3 tVec = position + map[index].position;
	double nx = tVec.x / (O_DIM * 1.0) - 0.5;
	double nz = tVec.z / (O_DIM * 1.0) - 0.5;
	double tVal = open_simplex_noise2(ctx, nx, nz);
	
	double hVal = 1.0 * open_simplex_noise2(ctx, nx * 1.0, nz * 1.0);
		   hVal+= 0.5 * open_simplex_noise2(ctx, nx * 10.0, nz * 10.0);
		   hVal += 0.25 * open_simplex_noise2(ctx, nx * 50.0, nz * 50.0);
		   hVal = powf(hVal, 4.0);
		   
	map[index].temp = tVal * 100.0;
		map[index].position.y = hVal * spacing * 40.0;
}

void Occulus::initMap() {
	for (int i = O_MIN; i < O_MAX; i++) {
		for (int j = O_MIN; j < O_MAX; j++) {
			this->map.push_back(Sector(j*spacing, 0.0f, i*spacing));
			mapNoise(map.size() - 1);
		}
	}
}

void Occulus::updateMap() {
	if (map.size()) {
		for (int i = 0; i < O_DIM; i++) {
			for (int j = 0; j < O_DIM; j++) {
				mapNoise(i*O_DIM + j);
			}
		}
	}
}

void Occulus::update(vec3 pos, vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps) {
	position.x = pos.x;
	position.z = pos.z;
	updateMap();
	draw(vertices, normals, uvs, temps);
	smoothShading(normals);
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
			vec4 n2 = calcNormal(p4, p2, p3);

			nIndex.push_back(n1);
			nIndex.push_back(n2);

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
			vec4 p1 = vec4(this->position + map[i*O_DIM + j].position, 1.0);
			vec4 p2 = vec4(this->position + map[i*O_DIM + (j + 1)].position, 1.0);
			vec4 p3 = vec4(this->position + map[(i + 1)*O_DIM + j].position, 1.0);
			vec4 p4 = vec4(this->position + map[(i + 1)*O_DIM + (j + 1)].position, 1.0);
			float t1 = map[i*O_DIM + j].temp;
			float t2 = map[i*O_DIM + (j + 1)].temp;
			float t3 = map[(i + 1)*O_DIM + j].temp;
			float t4 = map[(i + 1)*O_DIM + (j + 1)].temp;
			vec4 n1 = calcNormal(p1, p3, p2);
			vec4 n2 = calcNormal(p4, p2, p3);

			nIndex[2 * (i*limit + j)] = n1;
			nIndex[2 * (i*limit + j) + 1] = n2;

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
			vertices[6 * (i*limit + j) + 5] = p2;
			temps[6 * (i*limit + j) + 5] = t2;
			normals[6 * (i*limit + j) + 5] = n2;
			vertices[6 * (i*limit + j) + 4] = p4;
			temps[6 * (i*limit + j) + 4] = t4;
			normals[6 * (i*limit + j) + 4] = n2;

		}
	}
}

vec4 Occulus::calcNormal(vec3 p1, vec3 p2, vec3 p3) {
	vec3 U = normalize(p2 - p1);
	vec3 V = normalize(p3 - p1);
	//vec3 normal = cross(U, V);
	vec3 normal;
	normal.x = U.y * V.z + U.z * V.y;
	normal.y = U.z * V.x - U.x * V.z;
	normal.z = U.x * V.y - U.y * V.x;
	return vec4(normalize(normal), 1.0f);
}

void Occulus::smoothShading(vector<vec4> &normals) {
	vector<vec4> nNorm;
	int limit = O_DIM - 1;
	int edge = limit - 1;

	vec4 n1;
	vec4 n2;
	vec4 n3;
	vec4 n4;
	vec4 n5;
	vec4 n6;

	for (int i = 0; i < limit; i++) {
		for (int j = 0; j < limit; j++) {
			// start by initializing n1
			n1 = nIndex[2 * (i * limit + j)];

			// start by initializing n2
			n2 = nIndex[2 * (i * limit + j)];
			n2 += nIndex[2 * (i * limit + j) + 1];

			// start by initializing n3
			n3 = nIndex[2 * (i * limit + j)];
			n3 += nIndex[2 * (i * limit + j) + 1];

			// start by initializing n5
			n5 = nIndex[2 * (i * limit + j) + 1];

			if (i != edge) {
				// add side to n3
				n3 += nIndex[2 * ((i + 1)*limit + j)];

				// add sides to n5
				n5 += nIndex[2 * ((i + 1)*limit + j)];
				n5 += nIndex[2 * ((i + 1)*limit + j) + 1];
			}
			if (j != edge) {
				// add sides to n5
				n5 += nIndex[2 * (i*limit + (j + 1))];
				n5 += nIndex[2 * (i*limit + (j + 1)) + 1];

				if (i != edge) {
					// add side to n5
					n5 += nIndex[2 * ((i + 1)*limit + (j + 1))];
				}
			}

			if (i) {
				// add sides to n1
				n1 += nIndex[2 * ((i - 1) * limit + j)];
				n1 += nIndex[2 * ((i - 1) * limit + j) + 1];

				// add sides to n2
				n2 += nIndex[2 * ((i - 1) * limit + j) + 1];

				if (j != edge) {
					// add sides to n2
					n2 += nIndex[2 * (i * limit + (j + 1))];
					n2 += nIndex[2 * ((i - 1) * limit + (j + 1))];
					n2 += nIndex[2 * ((i - 1) * limit + (j + 1)) + 1];
				}
				if (j) {
					// add sides to n1
					n1 += nIndex[2 * ((i - 1) * limit + (j - 1)) + 1];
					n1 += nIndex[2 * (i * limit + (j - 1))];
					n1 += nIndex[2 * (i * limit + (j - 1)) + 1];

					// add sides to n3
					n3 += nIndex[2 * (i * limit + (j - 1)) + 1];

					if (i != edge) {
						// add sides to n3
						n3 += nIndex[2 * ((i + 1) * limit + (j - 1))];
						n3 += nIndex[2 * ((i + 1) * limit + (j - 1)) + 1];
					}
				}
			}
			else {
				if (j != edge) {
					// add sides to n2
					n2 += nIndex[2 * (i * limit + (j + 1))];
				}
				if (j) {
					// add sides to n1
					n1 += nIndex[2 * (i * limit + (j - 1))];
					n1 += nIndex[2 * (i * limit + (j - 1)) + 1];

					// add sides to n3
					n3 += nIndex[2 * (i * limit + (j - 1)) + 1];
					n3 += nIndex[2 * ((i + 1) * limit + (j - 1))];
					n3 += nIndex[2 * ((i + 1) * limit + (j - 1)) + 1];
				}
			}

			vec3 temp;

			temp = vec3(n1.x, n1.y, n1.z);
			temp /= glm::length(temp);
			n1 = vec4(glm::normalize(temp), 1.0f);

			temp = vec3(n2.x, n2.y, n2.z);
			temp /= glm::length(temp);
			n2 = vec4(glm::normalize(temp), 1.0f);

			temp = vec3(n3.x, n3.y, n3.z);
			temp /= glm::length(temp);
			n3 = vec4(glm::normalize(temp), 1.0f);

			temp = vec3(n5.x, n5.y, n5.z);
			temp /= glm::length(temp);
			n5 = vec4(glm::normalize(temp), 1.0f);

			n6 = n2;
			n4 = n3;

			normals[6 * (i * limit + j)] = n1;
			normals[6 * (i * limit + j) + 1] = n2;
			normals[6 * (i * limit + j) + 2] = n3;
			normals[6 * (i * limit + j) + 3] = n4;
			normals[6 * (i * limit + j) + 4] = n5;
			normals[6 * (i * limit + j) + 5] = n6;
		}
	}
}

