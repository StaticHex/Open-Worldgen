#pragma once
#include<glm/glm.hpp>

struct Sector {
	glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
	float temp;
	const float size = 0.25;
	void init(float x, float y, float z) {
		position = glm::vec3(x, y, z);
	}
	void init(glm::vec3 pos) {
		position = pos;
	}
	void copy(Sector newSect) {
		position.y = newSect.position.y;
		temp = newSect.temp;
	}
};