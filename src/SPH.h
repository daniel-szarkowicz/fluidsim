#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/geometric.hpp>

#include <vector>
#include <math.h>

#include "SpatialGrid3.h"

class SPH3 {
	float smoothingRadius = 0.2f;
	std::vector<Sphere*> particles;
	SpatialGrid3 grid;

	//arg r is the distance between the particles
	float smoothingKernel(float r);
	float smoothingKernelDerived(float r);

	glm::vec3 downVec;
	float g = 9.81;
	float restingDensity;
	float k;

	void calculateDensities(); 
	void calculatePressure();
	glm::vec3 calculatePressureForce(const Sphere* defaultSp);
public:

	SPH3(std::vector<Sphere> sp, glm::vec4* bounds, float dimW, float dimH, float dimD, float r0 = 20, float k = 20) : downVec(glm::vec3(0, -1, 0)), restingDensity(r0), k(k) {}
	
	void step(float dt);

	const std::vector<Sphere*> getParticles() { return particles; }
};
