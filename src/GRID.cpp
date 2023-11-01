#include "GRID.h"


void Grid::addToCell(const glm::vec2& idx, Sphere& p) {
	cells[idx.y][idx.x].particles.push_back(&p);
	cells[idx.y][idx.x].particleCount++;
}

void Grid::removeFromCell(const glm::vec2& idx, const Sphere& p) {
	if (cells[idx.y][idx.x].particleCount == 0) return;
	for (int i = 0; i < cells[idx.y][idx.x].particleCount; ++i) {
		if (cells[idx.y][idx.x].particles[i] == &p) {
			cells[idx.y][idx.x].particles.erase(cells[idx.y][idx.x].particles.begin() + i);
			cells[idx.y][idx.x].particleCount--;
		}
	}
}

bool Grid::twoVecEq(const glm::vec2& v1, const glm::vec2& v2) {
	float eps = 0.0001;
	return (fabs(v1.x - v2.x) < eps) && (fabs(v1.y - v2.y) < eps);
}

glm::vec2 Grid::getIdx(const glm::vec2& wCoords) {
	int x = ceil((wCoords.x - origo.x) / dimW);
	int y = ceil((wCoords.y - origo.y) / dimH);

	return glm::vec2(x, y);
}

void Grid::addParticle(Sphere* sphere) {
	glm::vec2 gCoords = getIdx(sphere->center);
	addToCell(gCoords, *sphere);
	particlesRef[(size_t)sphere] = gCoords;
}

std::vector<Sphere*> Grid::getNeighbours(const glm::vec2& pos, Sphere& base) {
	std::vector<Sphere*> ngh = std::vector<Sphere*>();
	glm::vec2 gCoords = getIdx(pos);
	int xL = gCoords.x - 1, xR = gCoords.x + 1, yU = gCoords.y + 1, yD = gCoords.y - 1;
	
	if (gCoords.x == 0) xL = gCoords.x;
	else if (gCoords.x == numCellX - 1) xR = gCoords.x;
	if (gCoords.y == 0) yD = gCoords.y;
	else if (gCoords.y == numCellY - 1) yU = gCoords.y;

	for (int y = yD; y <= yU; ++y) {
		for (int x = xL; x <= xR; ++x) {
			for (auto* sp : cells[y][x].particles) {
				ngh.push_back(sp);
			}
		}
	}

	return ngh;
}

void Grid::updateParticle(Sphere& p) {
	glm::vec2 gCoords = getIdx(p.center);
	if (twoVecEq(particlesRef[(size_t)&p], gCoords)) return;
	removeFromCell(particlesRef[(size_t)&p], p);
	addToCell(gCoords, p);
	particlesRef[(size_t)&p] = gCoords;
}