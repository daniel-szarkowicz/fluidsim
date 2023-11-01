#include "SpatialGrid.h"

void SpatialGrid2::addParticle(Particle2* particle) {
	glm::vec2 indicies = getCell(particle->pos);
	cells[indicies.y][indicies.x].particles.push_back(particle);
	cells[indicies.y][indicies.x].particleCount++;
	particlesRef[(unsigned int)particle] = indicies;
}

glm::vec2 SpatialGrid2::getCell(const glm::vec2& coords) {
	int x = ceil(coords.x / dimW);
	int y = ceil(coords.y / dimH);

	return glm::vec2(x, y);
}

std::vector< Particle2*> SpatialGrid2::getNeighbours(const glm::vec2& pos) {
	glm::vec2 indicies = getCell(pos);
	std::vector< Particle2*> particles = std::vector< Particle2*>();
	int xL = indicies.x - 1, xR = indicies.x + 1, yD = indicies.y - 1, yU = indicies.y + 1;
	if (indicies.x == 0) {
		//If we are at the bottom left of the grid
		if (indicies.y == 0) { yD = indicies.y; }
		//If we are at the top left of the grid
		else if (indicies.y == cells.size() - 1) { yU = indicies.y; }
		xL = indicies.x;
	}else if (indicies.x == cells[0].size() - 1) {
		if (indicies.y == 0) { yD = indicies.y; }
		else if (indicies.y == cells.size() - 1) { yU = indicies.y; }
		xR = indicies.x;
	}

	for(int i = xL; i <= xR; ++i) {
		for (int j = yD; j <= yU; ++j) {
			if (j == indicies.y && i == indicies.x) continue;
			for (int k = 0; k < cells[j][i].particles.size(); ++k) particles.push_back(cells[j][i].particles[k]);
		}
	}

	return particles;
}

std::vector<Particle2*> SpatialGrid2::getParticles() {
	std::vector<Particle2*> ps = std::vector<Particle2*>();
	for (int i = 0; i < cells.size(); ++i) {
		for(int j = 0; j < cells[0].size(); ++j){
			ps.push_back(cells[i][j].particles[0]);
		}
	}

	return ps;
}

bool SpatialGrid2::floatEq(float f1, float f2) {
	float eps = 0.00001f;
	return std::fabs(f1 - f2) < eps;
}

bool SpatialGrid2::isParticleInCell(const glm::vec2& coords, const Particle2& p) {
	if (cells[coords.y][coords.x].particleCount == 0) return false;
	for (auto part : cells[coords.y][coords.x].particles) if (part == &p) return true;

	return false;
}


void SpatialGrid2::updateParticle(Particle2& p) {
	glm::vec2 newIdx = getCell(p.pos), oldIdx = particlesRef[(unsigned int)&p];
	if (areVectorsEqual(oldIdx, newIdx)) return;
	removeFromCell(oldIdx, p);
	addToCell(newIdx, p);
	particlesRef[(unsigned int)&p] = newIdx;
}

glm::vec2& SpatialGrid2::getParticleIdx(const Particle2& p) {
	return particlesRef[(unsigned int) & p];
}


bool SpatialGrid2::areVectorsEqual(const glm::vec2& v1, const glm::vec2& v2) {
	return floatEq(v1.x, v2.x) && floatEq(v2.y, v1.y);
}

void SpatialGrid2::removeFromCell(const glm::vec2& idx, const Particle2& p) {
	int i;
	for (i = 0; i < cells[idx.y][idx.x].particles.size(); ++i) if (cells[idx.y][idx.x].particles[i] == &p) break;
	if (i == cells[idx.y][idx.x].particles.size()) return;
	cells[idx.y][idx.x].particles.erase(cells[idx.y][idx.x].particles.begin() + i);
	cells[idx.y][idx.x].particleCount--;
}

void SpatialGrid2::addToCell(const glm::vec2& idx,Particle2& p) {
	cells[idx.y][idx.x].particles.push_back(&p);
	cells[idx.y][idx.x].particleCount++;
}


void SpatialGrid2::clearGrid() {
	for (int i = 0; i < numCellY; ++i) {
		for (int j = 0; j < numCellX; ++j) {
			cells[i][j].particleCount = 0;
			cells[i][j].particles.clear();
		}
	}
}