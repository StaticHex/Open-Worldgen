#include "Occulus.h"
using glm::clamp;
using std::make_tuple;

// Default Constructor
// @description:
// - Used to create a new view area centered at X:0.0, Y:0.0, Z:0.0
Occulus::Occulus() :
	spacing(Sector().size * 2.0),
	seaLevel(-0.1)
{
	position = vec3(0.0f, 0.0f, 0.0f);
	lPosition = position;
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
	position(vec3(x, y, z)),
	spacing(Sector().size * 2.0),
	seaLevel(-0.1)
{
	size = spacing * O_DIM * C_DIM;
	lPosition = position;
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
	position(pos),
	spacing(Sector().size * 2.0),
	seaLevel(-0.1)
{
	size = spacing * O_DIM;
	lPosition = position;
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
tuple<float, float> Occulus::mapNoise(int index) {
	vec3 tVec = position + map[index].position;
	float nx = tVec.x / (O_DIM * 1.0) - 0.5;
	float nz = tVec.z / (O_DIM * 1.0) - 0.5;
	float tVal = open_simplex_noise2(ctx, nx, nz);

	float hVal = 1.0 * open_simplex_noise2(ctx, nx * 10.0, nz * 10.0);
	hVal += 0.5 * open_simplex_noise2(ctx, nx * 20.0, nz * 20.0);
	hVal += 0.25 * open_simplex_noise2(ctx, nx * 50.0, nz * 30.0);
	hVal = glm::max(powf(hVal, 2.33334), 0.0f);
	hVal -= 0.25 * open_simplex_noise2(ctx, nx * 5.0, nz * 5.0);
	hVal *= 20.001;
	tVal = clamp(tVal * 100.0 - hVal*2.0, 0.0, 100.0);
	hVal = static_cast<int>(hVal*100.0) / 100.0;
	tVal = static_cast<int>(tVal*100.0) / 100.0;
	return make_tuple(hVal, tVal);
}

// Initialize Map Method
// @description
// - Initalizes the view area's map of sectors upon initial creation 
//   of the view area. This should never be run outside of the 
//   constructor function.
void Occulus::initMap() {
	for (int i = O_MIN; i < O_MAX; i++) {
		for (int j = O_MIN; j < O_MAX; j++) {
			Sector newSec = Sector();
			newSec.init(j*spacing, 0.0f, i*spacing);
			this->map.push_back(newSec);
			tuple<float, float> ht = mapNoise(map.size() - 1);
			map.back().position.y = get<0>(ht);
			map.back().temp = get<1>(ht);
		}
	}
}

// Update Map Method
// @description
// - Updates noise-based parameters for each sector when the update function 
//   is called.
void Occulus::updateMap() {
	vec3 dV = lPosition - position;
	lPosition = position;
	if (dV.x != 0.0 || dV.z != 0.0) {
		if (map.size()) {
			for (int i = 0; i < O_DIM; i++) {
				for (int j = 0; j < O_DIM; j++) {
					int idx = i*O_DIM + j;
					tuple<float, float> ht = mapNoise(idx);
					map[idx].position.y = get<0>(ht);
					map[idx].temp = get<1>(ht);
				}
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
void Occulus::update(vec3 pos, vector<float> &heights, vector<vec4> &normals, vector<float> &temps, vector<uvec3> &faces) {
	position.x = pos.x;
	position.z = pos.z;
	updateMap();
	draw(normals, temps, heights, faces);
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
	int indNum = 0;
	int limit = O_DIM - 1;
	std::map<int, int> indexMap;
	indexMap.clear(); // clear our map and get ready to init water

	std::map<int, int>::iterator it; // create an iterator to check for existence of keys in map

	for (int i = 0; i < limit; i++) {
		for (int j = 0; j < limit; j++) {
			// get index positions from global grid
			float ind1 = i*O_DIM + j;
			float ind2 = i*O_DIM + (j + 1);
			float ind3 = (i + 1)*O_DIM + j;
			float ind4 = (i + 1)*O_DIM + (j + 1);

			// create two faces
			uvec3 f1 = uvec3(0, 0, 0);
			uvec3 f2 = uvec3(0, 0, 0);

			vec3 p1 = map[ind1].position;
			vec3 p2 = map[ind2].position;
			vec3 p3 = map[ind3].position;
			vec3 p4 = map[ind4].position;

			vec3 n1 = calcNormal(p1, p3, p2);
			vec3 n2 = calcNormal(p4, p2, p3);

			// calculate the UV values for each point
			float xfac = j*1.0;
			float yfac = i*1.0;
			vec2 p1Uv = vec2(UVX_MIN + xfac, UVY_MIN + yfac);
			vec2 p2Uv = vec2(UVX_MAX + xfac, UVY_MIN + yfac);
			vec2 p3Uv = vec2(UVX_MIN + xfac, UVY_MAX + yfac);
			vec2 p4Uv = vec2(UVX_MAX + xfac, UVY_MAX + yfac);

			// Check for existence of ind1 in our map
			it = indexMap.find(ind1);
			if (it != indexMap.end())
			{
				// calculate the index
				int idx = it->second;

				// set properties for p1
				f1.x = idx;
				normals[idx].x += n1.x;
				normals[idx].y += n1.y;
				normals[idx].z += n1.z;
			}
			else {
				// add properties to p1 to arrays and then increment our index
				f1.x = indNum;
				vertices.push_back(vec4(p1, 1.0));
				indexes.push_back(ind1);
				normals.push_back(vec4(n1, 1.0));
				uvs.push_back(p1Uv);
				temps.push_back(map[ind1].temp);
				heights.push_back(map[ind1].position.y);
				indexMap.insert({ ind1, indNum });
				indNum++;
			}

			// Check for the existence of ind2 in our map
			it = indexMap.find(ind2);
			if (it != indexMap.end())
			{
				// calculate the index
				int idx = it->second;

				// set properties for p2
				f1.y = idx;
				normals[idx].x += n1.x;
				normals[idx].y += n1.y;
				normals[idx].z += n1.z;

				// set properties for p6
				f2.z = idx;
				normals[idx].x += n2.x;
				normals[idx].y += n2.y;
				normals[idx].z += n2.z;
			}
			else {
				// add properties to p2 to arrays
				f1.y = indNum;
				vertices.push_back(vec4(p2, 1.0));
				indexes.push_back(ind2);
				normals.push_back(vec4(n1, 1.0));
				temps.push_back(map[ind2].temp);
				heights.push_back(map[ind2].position.y);
				uvs.push_back(p2Uv);


				// add properties for p6 to arrays
				f2.z = indNum;
				normals[indNum].x += n2.x;
				normals[indNum].y += n2.y;
				normals[indNum].z += n2.z;

				// add index to map and increment index
				indexMap.insert({ ind2, indNum });
				indNum++;
			}

			// Check for the existence of ind3 in our map
			it = indexMap.find(ind3);
			if (it != indexMap.end())
			{
				// calculate the index
				int idx = it->second;

				// set properties for p3
				f1.z = idx;
				normals[idx].x += n1.x;
				normals[idx].y += n1.y;
				normals[idx].z += n1.z;

				// set properties for p4
				f2.x = idx;
				normals[idx].x += n2.x;
				normals[idx].y += n2.y;
				normals[idx].z += n2.z;
			}
			else {
				// add properties to p3 to arrays
				f1.z = indNum;
				vertices.push_back(vec4(p3, 1.0));
				indexes.push_back(ind3);
				normals.push_back(vec4(n1, 1.0));
				temps.push_back(map[ind3].temp);
				heights.push_back(map[ind3].position.y);
				uvs.push_back(p3Uv);

				// add properties for p6 to arrays
				f2.x = indNum;
				normals[indNum].x += n2.x;
				normals[indNum].y += n2.y;
				normals[indNum].z += n2.z;

				// increment index
				indexMap.insert({ ind3, indNum });
				indNum++;
			}

			// Check for existence of ind4 in our map
			it = indexMap.find(ind4);
			if (it != indexMap.end())
			{
				// calculate the index
				int idx = it->second;

				// set properties for p1
				f2.y = idx;
				normals[idx].x += n2.x;
				normals[idx].y += n2.y;
				normals[idx].z += n2.z;
			}
			else {
				// add properties to p1 to arrays and then increment our index
				f2.y = indNum;
				vertices.push_back(vec4(p4, 1.0));
				indexes.push_back(ind4);
				normals.push_back(vec4(n2, 1.0));
				uvs.push_back(p4Uv);
				temps.push_back(map[ind4].temp);
				heights.push_back(map[ind4].position.y);
				indexMap.insert({ ind4, indNum });
				indNum++;
			}

			// push our faces
			faces.push_back(f1);
			int temp = f2.x;
			f2.x = f2.y;
			f2.y = temp;
			faces.push_back(f2);
		}
	}

	// normalize vectors to accomplish smooth shading
	int normSize = normals.size();
	for (int i = 0; i < normSize; i++) {
		vec3 n = vec3(normals[i].x, normals[i].y, normals[i].z);
		n = glm::normalize(n);
		normals[i].x = n.x;
		normals[i].y = n.y;
		normals[i].z = n.z;
	}
}

// Draw Water Method
// @param
// - vertices: the location of the vertices making up our water
// - normals: the location of the normal map for our water
// - uvs: the location of the UV map for our water
// - faces: the location of our vertex indicies
void Occulus::drawWater(vector<vec4> &vertices, vector<vec2> &uvs, vector<uvec3>&faces) {
	int indNum = 0;
	int limit = O_DIM - 1;
	std::map<int, int> indexMap;
	std::map<int, int>::iterator it; // create an iterator to check for existence of keys in map

	for (int i = 0; i < limit; i++) {
		for (int j = 0; j < limit; j++) {
			// get index positions from global grid
			float ind1 = i*O_DIM + j;
			float ind2 = i*O_DIM + (j + 1);
			float ind3 = (i + 1)*O_DIM + j;
			float ind4 = (i + 1)*O_DIM + (j + 1);

			// create two faces
			uvec3 f1 = uvec3(0, 0, 0);
			uvec3 f2 = uvec3(0, 0, 0);

			vec3 p1 = map[ind1].position;
			vec3 p2 = map[ind2].position;
			vec3 p3 = map[ind3].position;
			vec3 p4 = map[ind4].position;

			p1.y = seaLevel;
			p2.y = seaLevel;
			p3.y = seaLevel;
			p4.y = seaLevel;

			// calculate the UV values for each point
			float xfac = j*1.0;
			float yfac = i*1.0;
			vec2 p1Uv = vec2(UVX_MIN + xfac, UVY_MIN + yfac);
			vec2 p2Uv = vec2(UVX_MAX + xfac, UVY_MIN + yfac);
			vec2 p3Uv = vec2(UVX_MIN + xfac, UVY_MAX + yfac);
			vec2 p4Uv = vec2(UVX_MAX + xfac, UVY_MAX + yfac);

			// Check for existence of ind1 in our map
			it = indexMap.find(ind1);
			if (it != indexMap.end())
			{
				// calculate the index
				int idx = it->second;

				// set properties for p1
				f1.x = idx;
			}
			else {
				// add properties to p1 to arrays and then increment our index
				f1.x = indNum;
				vertices.push_back(vec4(p1, 1.0));
				uvs.push_back(p1Uv);
				indexMap.insert({ ind1, indNum });
				indNum++;
			}

			// Check for the existence of ind2 in our map
			it = indexMap.find(ind2);
			if (it != indexMap.end())
			{
				// calculate the index
				int idx = it->second;

				// set properties for p2
				f1.y = idx;

				// set properties for p6
				f2.z = idx;
			}
			else {
				// add properties to p2 to arrays
				f1.y = indNum;
				vertices.push_back(vec4(p2, 1.0));
				uvs.push_back(p2Uv);


				// add properties for p6 to arrays
				f2.z = indNum;

				// add index to map and increment index
				indexMap.insert({ ind2, indNum });
				indNum++;
			}

			// Check for the existence of ind3 in our map
			it = indexMap.find(ind3);
			if (it != indexMap.end())
			{
				// calculate the index
				int idx = it->second;

				// set properties for p3
				f1.z = idx;

				// set properties for p4
				f2.x = idx;
			}
			else {
				// add properties to p3 to arrays
				f1.z = indNum;
				vertices.push_back(vec4(p3, 1.0));
				uvs.push_back(p3Uv);

				// add properties for p6 to arrays
				f2.z = indNum;

				// increment index
				indexMap.insert({ ind3, indNum });
				indNum++;
			}

			// Check for existence of ind4 in our map
			it = indexMap.find(ind4);
			if (it != indexMap.end())
			{
				// calculate the index
				int idx = it->second;

				// set properties for p1
				f2.y = idx;
			}
			else {
				// add properties to p1 to arrays and then increment our index
				f2.y = indNum;
				vertices.push_back(vec4(p4, 1.0));
				uvs.push_back(p4Uv);
				indexMap.insert({ ind4, indNum });
				indNum++;
			}

			// push our faces
			faces.push_back(f1);
			int temp = f2.x;
			f2.x = f2.y;
			f2.y = temp;
			faces.push_back(f2);
		}
	}
}

// Draw Method (Private)
// @param
// - normals: The location of the normal map for the view area
// - temps: The location of the temp map for the view area
// - heights: The location of the height map for the view area
// - faces: used to update normals, temps, and heights. 
// @description
// - This is the method called by the update function; only updates global
//   vector maps which have the potential to change between frames. Uses the faces
//   map to help calculate the vertex normals for each point
void Occulus::draw(vector<vec4> &normals, vector<float> &temps, vector<float> &heights, vector<uvec3> &faces) {
	int indNum = 0;
	int limit = O_DIM - 1;
	int normSize = normals.size();

	// Clear out normals array
	for (int i = 0; i < normSize; i++) {
		normals[i] = vec4(0.0f, 0.0f, 0.0f, 1.0);
		temps[i] = 0.0;
		heights[i] = 0.0;
	}

	for (int i = 0; i < faces.size(); i++) {

		// indicies mapping into our vectors
		int i1 = faces[i].x;
		int i2 = faces[i].y;
		int i3 = faces[i].z;

		// indicies mapping into our grid
		int ind1 = indexes[i1];
		int ind2 = indexes[i2];
		int ind3 = indexes[i3];

		// get our points
		vec3 p1 = map[ind1].position;
		vec3 p2 = map[ind2].position;
		vec3 p3 = map[ind3].position;

		// do a normal calculation
		vec3 n1 = calcNormal(p1, p3, p2);

		//update our normal array by adding the face normal
		normals[i1] += vec4(n1, 1.0);
		normals[i2] += vec4(n1, 1.0);
		normals[i3] += vec4(n1, 1.0);

		// update temperature and height map
		temps[i1] = map[ind1].temp;
		temps[i2] = map[ind2].temp;
		temps[i3] = map[ind3].temp;
		heights[i1] = map[ind1].position.y;
		heights[i2] = map[ind2].position.y;
		heights[i3] = map[ind3].position.y;
	}

	// normalize vectors to accomplish smooth shading
	for (int i = 0; i < normSize; i++) {
		vec3 n = vec3(normals[i].x, normals[i].y, normals[i].z);
		n = glm::normalize(n);
		normals[i].x = n.x;
		normals[i].y = n.y;
		normals[i].z = n.z;
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