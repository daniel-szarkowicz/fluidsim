#pragma once
#include <glm/vec2.hpp>


//Barebones, sides have to be parallel with the axes 
//TODO: rewrite with equations so any form is possible, not just parallel rectangles
struct BoundingBox2 {
	//order of the corners should be { left bottom -> right bottom -> right top -> left top }
	glm::vec2 corners[4];

	void cpyToCorners(const glm::vec2 vec[]) {
		for (int i = 0; i < 4; ++i) {
			corners[i] = vec[i];
		}
	}

	BoundingBox2() {}

	BoundingBox2(const BoundingBox2& b) {
		cpyToCorners(b.corners);
	}

	BoundingBox2(const glm::vec2 vec[]) { 
		cpyToCorners(vec);
	}

	bool collides(const glm::vec2 p) {
		if (p.x <= corners[0].x || p.x >= corners[2].x) return true;
		if (p.y <= corners[0].y || p.y >= corners[2].y) return true;
		
		return false;
	}
};