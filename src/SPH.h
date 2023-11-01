#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/geometric.hpp>

#include <vector>
#include <math.h>

#include "SpatialGrid3.h"

class SPH3 {
	float smoothingRadius;
	std::vector<Sphere*> particles;
	SpatialGrid3 grid;
	glm::vec4 bounds[8];

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
	glm::vec4 calculatePressureForce(const Sphere* defaultSp, int i);
	void checkBounds(Sphere* sp);
public:

	SPH3(std::vector<Sphere>& sp, glm::vec4* bounds, float dimW, float dimH, float dimD, float r0 = 20, float k = 20, float gravity = 9.81, float smoothingRadius = 0.2)
		: downVec(glm::vec4(0, -1, 0, 1)), restingDensity(r0), k(k), g(gravity), smoothingRadius(smoothingRadius){

		for (int i = 0; i < 8; ++i) this->bounds[i] = bounds[i];

		density = std::vector<float>(sp.size());
		mass = std::vector<float>(sp.size());
		pressure = std::vector<float>(sp.size());

		grid = SpatialGrid3(bounds, dimW, dimH, dimD);
		for (auto& s : sp) {
			particles.push_back(&s);
			grid.addParticle(&s);
		}
	}
	
	void step(float dt);

	const std::vector<Sphere*> getParticles() { return particles; }
};
