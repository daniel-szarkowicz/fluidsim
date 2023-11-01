#pragma once
#include <vector>
#include <math.h>
#include <iostream>
#include <map>

#include <glm/vec2.hpp>


#include "BoundingBox.h"
#include "Particle.h"

class SpatialGrid2 {
	//A class only struct which counts how many patricles are in this cell
	//and a reference to the particles that are in it
	struct cell {
		int particleCount;
		std::vector<Particle2* > particles;
		cell(): particleCount(0) { particles = std::vector<Particle2*>(); }
	};
	
	
	//The area where the grid is working on
	BoundingBox2 bounds;
	//The dimensions of a grid cell
	float dimW, dimH;
	//The number of cells in each axis
	int numCellX, numCellY;

	//We store all the references of the stored particles for fast access
	//The key is the memory address of the particle and the value is the indicies of the cells matrix
	std::map<unsigned int, glm::vec2> particlesRef;

	//The cells which make up our grid
	std::vector<std::vector<cell> > cells;
	//Takes the world coordinates and returns the indicies to the cell that projects to that area of the world
	glm::vec2 getCell(const glm::vec2& coords);

	//Takes two floats and returns true if they are similiar between an eps range
	bool floatEq(float f1, float f2);
	
	//Returns true if the particle is in the cell of coords 
	//false otherwise
	bool isParticleInCell(const glm::vec2& coords, const Particle2& p);
	
	//Returns true if two vectors are the same (they hold the same values)
	bool areVectorsEqual(const glm::vec2& v1, const glm::vec2& v2);

	//Removes the particle from the cell at idx indicies
	void removeFromCell(const glm::vec2& idx, const Particle2& p);

	//Add particle p to the cell at idx indicies 
	void addToCell(const glm::vec2& idx,Particle2& p);
public:
	SpatialGrid2() {}
	SpatialGrid2(BoundingBox2 bounds, float dimW, float dimH) : bounds(bounds), dimW(dimW), dimH(dimH) {
		//We take the ceiling to have one extra, to not run out of bounds
		numCellX = ceil(abs(abs(bounds.corners[0].x) - abs(bounds.corners[1].x)) / dimW);
		numCellY = ceil(abs(abs(bounds.corners[0].y) - abs(bounds.corners[3].y)) / dimW);
		cells = std::vector<std::vector<cell>>(numCellY); 
		
		for (auto& c : cells) c = std::vector<cell>(numCellX);
	}
	
	//We add a particle to the grid
	void addParticle(Particle2* particle);
	//Takes the world coordinates as paramater and returns those which are considered neighbours based on the grid
	std::vector<Particle2*> getNeighbours(const glm::vec2& pos);
	
	//Returns the stored particles
	std::vector<Particle2*> getParticles();

	//Updates the storage of particle p
	void updateParticle(Particle2& p);
	
	//Returns the indicies of the particle p
	glm::vec2& getParticleIdx(const Particle2& p);

	//Clears the grid
	void clearGrid();
};
