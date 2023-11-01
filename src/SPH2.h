#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/random.hpp>

#include <vector>
//#include <math.h>
#include <algorithm>

#include "SpatialGrid3.h"
#include "GRID.h"

class SPH2 {
	float smoothingRadius;
	std::vector<Sphere*> particles;
	//SpatialGrid3 grid;
	Grid grid;
	glm::vec4 bounds[4];

	float g;
	float restingDensity;
	float k;

	//arg r is the distance between the particles
	float smoothingKernel(float r);
	float smoothingKernelDerived(float r);

	glm::vec4 downVec;
	std::vector<float> density;
	std::vector<float> mass;
	std::vector<float> pressure;


	void calculateDensities();
	void calculatePressure();
	glm::vec4 calculatePressureForce( Sphere* defaultSp, int i);
	void checkBounds(Sphere* sp);
public:

	SPH2(std::vector<Sphere>& sp, glm::vec4* bounds, float dimW, float dimH, float dimD, float r0 = 2.75, float k = 0.7, float gravity = 8, float smoothingRadius = 0.4)
		: downVec(glm::vec4(0, -1, 0, 0)), restingDensity(r0), k(k), g(gravity), smoothingRadius(smoothingRadius) {

		for (int i = 0; i < 4; ++i) this->bounds[i] = bounds[i];

		density = std::vector<float>(sp.size());
		mass = std::vector<float>(sp.size());
		pressure = std::vector<float>(sp.size());

		grid = Grid(bounds, dimW, dimH);
		for (auto& s : sp) {
			particles.push_back(&s);
			grid.addParticle(&s);
		}
	}

	void step(float dt);

	const std::vector<Sphere*> getParticles() { return particles; }
};