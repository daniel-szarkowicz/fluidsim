#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include "Particle.h"

#include <vector>
#include <map>

class Grid {
	struct cell {
		int particleCount;
		std::vector<Sphere* > particles;
		cell() : particleCount(0) { particles = std::vector<Sphere*>(); }
	};

	float dimW, dimH;
	int numCellX, numCellY;
	glm::vec2 origo;
	glm::vec2 bounds[4];

	std::map<size_t, glm::vec2> particlesRef;

	std::vector<std::vector<cell>> cells;
	void addToCell(const glm::vec2& idx, Sphere& p);
	void removeFromCell(const glm::vec2& idx, const Sphere& p);
	bool twoVecEq(const glm::vec2& v1, const glm::vec2& v2);
public:
	Grid() {}
	Grid(glm::vec4* bounds, float dimW, float dimH): dimW(dimW), dimH(dimH) {
		for (int i = 0; i < 4; ++i) {
			this->bounds[i].x = bounds[i].x;
			this->bounds[i].y = bounds[i].y;
 		}

		numCellX = ceil((bounds[1].x - bounds[0].x) / dimW) + 1;
		numCellY = ceil((bounds[2].y - bounds[0].y) / dimH) + 1;

		origo = bounds[0];

		cells = std::vector<std::vector<cell>>(numCellY);
		for (int i = 0; i < numCellY; ++i) cells[i] = std::vector<cell>(numCellX);
	}

	glm::vec2 getIdx(const glm::vec2& wCoord);
	void addParticle(Sphere* sphere);
	std::vector<Sphere*> getNeighbours(const glm::vec2& pos, Sphere& base);
	void updateParticle(Sphere& p);
};