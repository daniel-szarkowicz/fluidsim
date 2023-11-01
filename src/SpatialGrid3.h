#pragma once
#include <vector>
#include <map>
#include <math.h>

#include <glm/vec3.hpp>

#include "Particle.h"


//Works on the same principle as SpatialGrid2 so not much needs commenting
class SpatialGrid3 {
	struct cell {
		int particleCount;
		std::vector<Sphere* > particles;
		cell() : particleCount(0) { particles = std::vector<Sphere*>(); }
	};

	glm::vec4 bounds[8];
	float dimW, dimH, dimD;
	int numCellX, numCellY, numCellZ;

	std::map<size_t, glm::vec3> particleRef;


	//indexing of the cells
	//cells[y][x][z]
	std::vector<std::vector<std::vector<cell>>> cells;
	glm::vec3 getCell(const glm::vec4& coords);
	glm::vec4 gridOrigin;

	bool floatEq(float f1, float f2);
	bool areVectorsEqual(const glm::vec3& v1, const glm::vec3& v2);
	void removeFromCell(const glm::vec3& idx, const Sphere& p);
	void addToCell(const glm::vec3& idx, Sphere& p);
public:
	SpatialGrid3() {}
	//Order of bounds is as follows
	//{bottom left front -> bottom right front -> top left front -> top right front}
	//{bottom left back -> bottom right back -> top left back -> top right back}
	SpatialGrid3(glm::vec4* bounds, float dimW, float dimH, float dimD): dimW(dimW), dimH(dimH), dimD(dimD) {
		for (int i = 0; i < 4; ++i) this->bounds[i] = bounds[i];

		numCellX = ceil((abs(bounds[0].x - bounds[1].x)) / dimW) + 1;
		numCellY = ceil((abs(bounds[0].y - bounds[3].y)) / dimH) + 1;
		//numCellZ = ceil((abs(bounds[0].z - bounds[4].z)) / dimD);
		numCellZ = 1;

		gridOrigin = bounds[0];

		cells = std::vector<std::vector<std::vector<cell>>>(numCellY);

		for (auto& cx : cells) {
			cx = std::vector<std::vector<cell>>(numCellX);
			for (auto& cz : cx) {
				cz = std::vector<cell>(numCellZ);
			}
		}
	}

	void addParticle(Sphere* particle);
	std::vector<Sphere*> getNeighbours(const glm::vec4& pos);
	std::vector<Sphere*> getParticles();
	void updateParticle(Sphere& p);
	glm::vec3& getParticleIdx(const Sphere& p);
	void clearGrid();
};