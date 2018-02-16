#include "Occulus.h"
using glm::clamp;

// Default Constructor
// @description:
// - Used to create a new view area centered at X:0.0, Y:0.0, Z:0.0
Occulus::Occulus() :
	spacing(Sector().size * 2.0)
{
	position = vec3(0.0f, 0.0f, 0.0f);
	size = spacing * O_DIM * C_DIM;
	open_simplex_noise(77374, &ctx);
	initMap();
}

// Float Position Constructor
// @Param
// - x: the x position of the center of the view area
// - y: the y position of the center of the view area
// - z: the z position of the center of the view area
// @description
// - Uses passed in x, y, and z coordinates to create a new
//   view area centered at hte specified location
Occulus::Occulus(float x, float y, float z) :
	spacing(Sector().size * 2.0)
{
	position = vec3(x, y, z);
	size = spacing * O_DIM * C_DIM;
	open_simplex_noise(77374, &ctx);
	initMap();
}

// Vector Position Constructor
// @param
// - pos: the position of the center of the view area
// @description
// - uses the passed vector as the center position for the 
//  view area
Occulus::Occulus(vec3 pos) :
	position(vec3(pos.x, pos.y, pos.z)),
	spacing(Sector().size * 2.0)
{
	size = spacing * O_DIM;
	open_simplex_noise(77374, &ctx);
	initMap();
}

// MapNoise Method
// @param
// - index: the sector index to map noise to
// @description
// - Uses open simplex to generate noise values for temperature
//   and height, multiplies these values, and sets them as the height
//   and temperature for the sector at the passed index.
void Occulus::mapNoise(int index) {
	vec3 tVec = position + map[index].position;
	double nx = tVec.x / (O_DIM * 1.0) - 0.5;
	double nz = tVec.z / (O_DIM * 1.0) - 0.5;
	double tVal = open_simplex_noise2(ctx, nx, nz);
	
	double hVal = 1.0 * open_simplex_noise2(ctx, nx * 10.0, nz * 10.0);
		   hVal+= 0.5 * open_simplex_noise2(ctx, nx * 20.0, nz * 20.0);
		   hVal += 0.25 * open_simplex_noise2(ctx, nx * 50.0, nz * 30.0);
		   hVal = glm::max(powf(hVal, 2.33334), 0.0f);
		   hVal *= 20.001;
		   tVal = clamp(tVal * 100.0 - hVal*2.0, 0.0, 100.0);
	map[index].position.y = hVal;
	map[index].temp = tVal;
}

// Initialize Map Method
// @description
// - Initalizes the view area's map of sectors upon initial creation 
//   of the view area. This should never be run outside of the 
//   constructor function.
void Occulus::initMap() {
	for (int i = O_MIN; i < O_MAX; i++) {
		for (int j = O_MIN; j < O_MAX; j++) {
			this->map.push_back(Sector(j*spacing, 0.0f, i*spacing));
			mapNoise(map.size() - 1);
		}
	}
}

// Update Map Method
// @description
// - Updates noise-based parameters for each sector when the update function 
//   is called.
void Occulus::updateMap() {
	if (map.size()) {
		for (int i = 0; i < O_DIM; i++) {
			for (int j = 0; j < O_DIM; j++) {
				mapNoise(i*O_DIM + j);
			}
		}
	}
}

// Update Method
// @param
// - pos: the new position of the occulus
// - heights: the location of the height map for the view area
// - normals: the location of the normal map for the view area
// - temps:   the location of the temperature map for the view area
// @description
// - Updates the position of the view area, calls the updateMap() method ot update the height map
//   and temperature map, and then calls the draw and smooth shading functions.
void Occulus::update(vec3 pos, vector<float> &heights, vector<vec4> &normals, vector<float> &temps) {
	position.x = pos.x;
	position.z = pos.z;
	updateMap();
	draw(normals, temps, heights);
	smoothShading(normals);
}

// Draw Method (Public)
// @param
// - vertices: location of the vertex map for the view area
// - normals: location of the normal map for the view area
// - uvs: location of the uv map for the view area
// - temps: location of the temperature map for the view area
// - heights: location of the height map for the view area
// - faces: location of the face map for the view area
// @description
// - Public draw function, called when program is first loaded and used to initialize 
//   global map vectors. This function is not called by update and should not be 
//   called every frame as doing so would result in a large amount of redundant 
//   calculations.
void Occulus::draw(vector<vec4> &vertices, vector<vec4> &normals, vector<vec2> &uvs,
	vector<float> &temps, vector<float> &heights, vector<uvec3> &faces) {
	int fNum = 0;
	int limit = O_DIM - 1;
	for (int i = 0; i < limit; i++) {
		for (int j = 0; j < limit; j++) {
			// calculate initial vertex positions
			vec4 p1 = vec4(map[i*O_DIM + j].position, 1.0);
			vec4 p2 = vec4(map[i*O_DIM + (j + 1)].position, 1.0);
			vec4 p3 = vec4(map[(i + 1)*O_DIM + j].position, 1.0);
			vec4 p4 = vec4(map[(i + 1)*O_DIM + (j + 1)].position, 1.0);

			// grab the temperatures for each point from our sector map
			float t1 = map[i*O_DIM + j].temp;
			float t2 = map[i*O_DIM + (j + 1)].temp;
			float t3 = map[(i + 1)*O_DIM + j].temp;
			float t4 = map[(i + 1)*O_DIM + (j + 1)].temp;

			// calculate the face normals for our triangles
			vec4 n1 = calcNormal(p1, p3, p2);
			vec4 n2 = calcNormal(p4, p2, p3);

			// calculate the UV values for each point
			vec2 p1Uv = vec2(UVX_MIN, UVY_MIN);
			vec2 p2Uv = vec2(UVX_MAX, UVY_MIN);
			vec2 p3Uv = vec2(UVX_MIN, UVY_MAX);
			vec2 p4Uv = vec2(UVX_MAX, UVY_MAX);

			uvs.push_back(p1Uv);
			uvs.push_back(p2Uv);
			uvs.push_back(p3Uv);
			uvs.push_back(p3Uv);
			uvs.push_back(p4Uv);
			uvs.push_back(p2Uv);

			nIndex.push_back(n1);
			nIndex.push_back(n2);
			vertices.push_back(p1);
			heights.push_back(p1.y);
			temps.push_back(t1);
			normals.push_back(n1);
			vertices.push_back(p2);
			heights.push_back(p2.y);
			temps.push_back(t2);
			normals.push_back(n1);
			vertices.push_back(p3);
			heights.push_back(p3.y);
			temps.push_back(t3);
			normals.push_back(n1);
			faces.push_back(vec3(fNum, fNum + 1, fNum + 2));
			fNum += 3;

			vertices.push_back(p3);
			heights.push_back(p3.y);
			temps.push_back(t3);
			normals.push_back(n2);
			vertices.push_back(p4);
			heights.push_back(p4.y);
			temps.push_back(t4);
			normals.push_back(n2);
			vertices.push_back(p2);
			heights.push_back(p2.y);
			temps.push_back(t2);
			normals.push_back(n2);
			faces.push_back(vec3(fNum, fNum + 1, fNum + 2));
			fNum += 3;
		}
	}
}

// Draw Method (Private)
// @param
// - normals: The location of the normal map for the view area
// - temps: The location of the temp map for the view area
// - heights: The location of the height map for the view area
// @description
// - This is the method called by the update function; only udpateds globale
//   vector maps which have the potential to change between frames
void Occulus::draw(vector<vec4> &normals, vector<float> &temps, vector<float> &heights) {

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

			heights[6 * (i*limit + j)] = p1.y;
			temps[6 * (i*limit + j)]= t1;
			normals[6 * (i*limit + j)] = n1;
			heights[6 * (i*limit + j) + 1] = p2.y;
			temps[6 * (i*limit + j) + 1] = t2;
			normals[6 * (i*limit + j) + 1] = n1;
			heights[6 * (i*limit + j) + 2] = p3.y;
			temps[6 * (i*limit + j) + 2] = t3;
			normals[6 * (i*limit + j) + 2] = n1;

			heights[6 * (i*limit + j) + 3] = p3.y;
			temps[6 * (i*limit + j) + 3] = t3;
			normals[6 * (i*limit + j) + 3] = n2;
			heights[6 * (i*limit + j) + 4] = p4.y;
			temps[6 * (i*limit + j) + 4] = t4;
			normals[6 * (i*limit + j) + 4] = n2;
			heights[6 * (i*limit + j) + 5] = p2.y;
			temps[6 * (i*limit + j) + 5] = t2;
			normals[6 * (i*limit + j) + 5] = n2;
		}
	}
}

// Calculate Normal Method
// @param
// - p1: point one of our triangle
// - p2: point two of our triangle
// - p3: point three of our triangle
// @description
// - takes in three points which consist a triangle and calculates
//   the face normal for that triangle based on the position of
//   each point.
vec4 Occulus::calcNormal(vec3 p1, vec3 p2, vec3 p3) {
	vec3 U = normalize(p2 - p1);
	vec3 V = normalize(p3 - p1);
	return vec4(cross(U, V), 1.0f);
}

// Smooth Shading Method
// @param
// - normals: The normal map for the view area
// @description
// - Takes in the normal map and averages all normals
//   at each vertex resulting in the reduction of the 
//   appearance of edges for each surface.
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

