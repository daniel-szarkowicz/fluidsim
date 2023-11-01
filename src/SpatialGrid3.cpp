#include "SpatialGrid3.h"

glm::vec3 SpatialGrid3::getCell(const glm::vec4& coords) {
	glm::vec4 gridCoords = glm::vec4(coords.x - gridOrigin.x, coords.y - gridOrigin.y, gridOrigin.z - coords.z, 1);
	int i = ceil(gridCoords.x / dimW);
	int j = ceil(gridCoords.y / dimH);
	int k = ceil(gridCoords.z / dimD);

	return glm::vec3(i, j, k);
}

bool SpatialGrid3::floatEq(float f1, float f2) { return fabs(f1 - f2) < 0.0001f; }

bool SpatialGrid3::areVectorsEqual(const glm::vec3& v1, const glm::vec3& v2) {
	return floatEq(v1.x, v2.x) && floatEq(v1.y, v2.y) && floatEq(v1.z, v2.z);
}

void SpatialGrid3::removeFromCell(const glm::vec3& idx, const Sphere& p) {
	if (cells[idx.y][idx.x][idx.z].particleCount == 0) return;
	int i;
	for (i = 0; i < cells[idx.y][idx.x][idx.z].particles.size(); ++i) if (cells[idx.y][idx.x][idx.z].particles[i] == &p) break;
	if (i == cells[idx.y][idx.x][idx.z].particleCount) return;
	cells[idx.y][idx.x][idx.z].particles.erase(cells[idx.y][idx.x][idx.z].particles.begin() + i);
	cells[idx.y][idx.x][idx.z].particleCount--;
}

void SpatialGrid3::addToCell(const glm::vec3& idx, Sphere& p) {
	cells[idx.y][idx.x][idx.z].particles.push_back(&p);
	cells[idx.y][idx.x][idx.z].particleCount++;
}

void SpatialGrid3::addParticle(Sphere* particle) {
	glm::vec3 idx = getCell(particle->center);
	addToCell(idx, *particle);
	particleRef[(unsigned int)particle] = idx;
}

std::vector<Sphere*> SpatialGrid3::getNeighbours(const glm::vec4& pos) {
	std::vector<Sphere*> spheres = std::vector<Sphere*>();
	glm::vec3 idx = getCell(pos);
	int xL = idx.x - 1, xR = idx.x + 1, yU = idx.y + 1, yD = idx.y - 1, zF = idx.z - 1, zB = idx.z + 1;

	if (idx.x == 0) xL = idx.x;
	else if (idx.x == numCellX) xR = idx.x;
	if (idx.y == 0) { yD = idx.y; }
	else if (idx.y == cells.size() - 1) { yU = idx.y; }
	if (idx.z == 0) { zF = idx.z; }
	else if (idx.z == numCellZ - 1) { zB = idx.z; }

	for (int j = yD; j <= yU; ++j) {
		for (int i = xL; i <= xR; ++i) {
			//for (int k = zF; k <= zB; ++k) {
				for (auto sp : cells[j][i][idx.z].particles) spheres.push_back(sp);
			//}
		}
	}

	return spheres;
}

std::vector<Sphere*> SpatialGrid3::getParticles() {
	std::vector<Sphere*> spheres = std::vector<Sphere*>();
	for (auto y : cells)
		for (auto x : y)
			for (auto z : x)
				for (auto sp : z.particles) spheres.push_back(sp);


	return spheres;
}

void SpatialGrid3::updateParticle(Sphere& p) {
	glm::vec3 newIdx = getCell(p.center), oldIdx = particleRef[(unsigned int)&p];
	if (areVectorsEqual(newIdx, oldIdx)) return;
	removeFromCell(oldIdx, p);
	addToCell(newIdx, p);
	particleRef[(unsigned int)&p] = newIdx;
}

glm::vec3& SpatialGrid3::getParticleIdx(const Sphere& p) {
	return particleRef[(unsigned int)&p];
}

void SpatialGrid3::clearGrid() {
	for (auto y : cells) {
		for (auto x : y) {
			for (auto z : x) {
				z.particleCount = 0;
				z.particles.clear();
			}
		}
	}
}