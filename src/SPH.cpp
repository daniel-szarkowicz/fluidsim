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

void SPH3::calculateDensities() {
	std::vector<Sphere*> sps;
	for (int i = 0; i < particles.size(); ++i) {
		sps = grid.getNeighbours(particles[i]->center);
		density[i] = 0;
		for (auto it : sps) {
			density[i] += smoothingKernel(glm::length(particles[i]->center - it->center));
		}
	}
}

void SPH3::calculatePressure() {
	for (int i = 0; i < particles.size(); ++i) pressure[i] = k * (pressure[i] - restingDensity);
}

glm::vec4 SPH3::calculatePressureForce(const Sphere* defaultSp, int idx) {
	glm::vec4 res = glm::vec4(0,0,0,1), dir;
	float distance;
	std::vector<Sphere*> neighbours = grid.getNeighbours(defaultSp->center);
	for (int i = 0; i < particles.size(); ++i) {
		if (defaultSp == particles[i]) continue;
		distance = glm::length(defaultSp->center - particles[i]->center);
		dir = (defaultSp->center - particles[i]->center) / distance;
		res -= (pressure[i] + pressure[idx]) / (density[i] * 2) * dir * smoothingKernelDerived(distance);
	}

	return res;
}

void SPH3::checkBounds(Sphere* sp) {
	if (sp->center.x < bounds[0].x + sp->radius) {
		sp->center.x = bounds[0].x + sp->radius;
		sp->velocity.x *= -1;
	}else if (sp->center.x > bounds[1].x - sp->radius) {
		sp->center.x = bounds[0].x - sp->radius;
		sp->velocity.x *= -1;
	}
	if (sp->center.y < bounds[0].y + sp->radius) {
		sp->center.y = bounds[0].y + sp->radius;
		sp->velocity.y *= -1;
	}else if (sp->center.y > bounds[0].y - sp->radius) {
		sp->center.y = bounds[0].y - sp->radius;
		sp->velocity.y *= -1;
	}
	if (sp->center.z < bounds[0].z + sp->radius) {
		sp->center.z = bounds[0].z + sp->radius;
		sp->velocity.z *= -1;
	}else if (sp->center.z > bounds[0].z - sp->radius) {
		sp->center.z = bounds[0].z - sp->radius;
		sp->velocity *= -1;
	}
}


void SPH3::step(float dt) {
	calculateDensities();
	calculatePressure();
#pragma omp parallel for 
	for (int i = 0; i < particles.size(); ++i) {
		particles[i]->velocity += downVec * g * dt;
		glm::vec4 Fp = calculatePressureForce(particles[i], i);
		particles[i]->velocity += Fp / density[i]* dt;
	}
#pragma omp parallel for 
	for (int i = 0; i < particles.size(); ++i) {
		particles[i]->center += particles[i]->velocity  * dt;
		checkBounds(particles[i]);
	}
}