#include <iostream>

#include "SPH.h"

float SPH3::smoothingKernel(float r) {
	float q = r / smoothingRadius, cond;
	if (q >= 0.0f && q <= 1.0f) cond = 1 + 3 / 2 * pow(q, 2) + 3 / 4 * pow(q, 3);
	else if (q <= 2.0f && q >= 1.0f) cond = 1 / 4 * pow(2 - q, 3);
	else return 0.0f;
	return 1 / (pow(smoothingRadius, 3) * glm::pi<float>()) * cond;
}

float SPH3::smoothingKernelDerived(float r) {
	float cond, q = r / smoothingRadius;
	if (q >= 0.0f && q <= 1.0f) cond = (3 * r * (4 * smoothingRadius + 3 * r)) / (4 * pow(smoothingRadius, 3));
	else if (q <= 2.0f && q >= 1.0f) cond = -3/(4*smoothingRadius)*pow((2-r/smoothingRadius), 2);
	else return 0.0f;

	return 1 / (pow(smoothingRadius, 3) * glm::pi<float>()) * cond;
}

SPH3::SPH3(std::vector<Sphere> sp, glm::vec4* bounds, float dimW, float dimH, float dimD, float r0, float k) {
	grid = SpatialGrid3(bounds, dimW, dimH, dimD);
	for (auto s : sp) {
		particles.push_back(&s);
		grid.addParticle(&s);
	}
}

void SPH3::calculateDensities() {
	std::vector<Sphere*> sps;
	for (auto sphere : particles) {
		sps = grid.getNeighbours(sphere->center);
		sphere->density = 0;
		for (auto it : sps) {
			if (it == sphere) continue;
			sphere->density += smoothingKernel(glm::length(sphere->center - it->center));
		}
	}
}

void SPH3::calculatePressure() {
	for (auto sphere : particles) sphere->pressure = k * (sphere->pressure - restingDensity);
}

glm::vec3 SPH3::calculatePressureForce(const Sphere* defaultSp) {
	glm::vec3 res = glm::vec3(0,0,0), dir;
	float distance;
	std::vector<Sphere*> neighbours = grid.getNeighbours(defaultSp->center);
	for (auto sp : neighbours) {
		distance = glm::length(defaultSp->center - sp->center);
		dir = (defaultSp->center - sp->center) / distance;
		res -= (sp->pressure + defaultSp->pressure) / (sp->density * 2) * dir * smoothingKernelDerived(glm::length(defaultSp->center - sp->center));
	}

	return res;
}


void SPH3::step(float dt) {
	calculateDensities();
	calculatePressure();
	for (auto sphere : particles) {
		sphere->velocity += downVec * g * dt;
		glm::vec3 Fp = calculatePressureForce(sphere);
		sphere->velocity += Fp / sphere->density * dt;
	}

	for (auto sphere : particles) {
		sphere->center += sphere->velocity  * dt;

	}
}