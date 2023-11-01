#include "SPH2.h"


//float SPH2::smoothingKernel(float r) {
//	float vol = glm::pi<float>() * pow(smoothingRadius, 8) / 4;
//	float val = std::max((float)0, smoothingRadius * smoothingRadius - r * r);
//	return val * val * val;
//}

float SPH2::smoothingKernel(float r) {
	if (r >= smoothingRadius) return 0;
	float volume = (glm::pi<float>() * pow(smoothingRadius, 4)) / 6;
	return (smoothingRadius - r) * (smoothingRadius - r) / volume;
}

//float SPH2::smoothingKernelDerived(float r) {
//	if (r >= smoothingRadius) return 0;
//	float f = smoothingRadius * smoothingRadius - r * r;
//	float scale = -24 / (glm::pi<float>() * pow(smoothingRadius, 8));
//	return scale * r * f * f;
//}

float SPH2::smoothingKernelDerived(float r) {
	if (r >= smoothingRadius) return 0;
	float scale = 12 / (pow(smoothingRadius, 4) * glm::pi<float>());
	return (r - smoothingRadius) * scale;
}

void SPH2::calculateDensities() {
	std::vector<Sphere*> sps;
	//sps = particles;
#pragma omp parallel for 
	for (int i = 0; i < particles.size(); ++i) {
		sps = grid.getNeighbours(glm::vec2(particles[i]->center.x, particles[i]->center.y), *particles[i]);
		density[i] = 0;
		for (auto it : sps) {
			density[i] += smoothingKernel(glm::length(particles[i]->center - it->center)/* - particles[i]->radius - it->radius*/);
		}
	}
}

void SPH2::calculatePressure() {
#pragma omp parallel for 
	for (int i = 0; i < particles.size(); ++i) pressure[i] = k * (density[i] - restingDensity);
}

glm::vec4 SPH2::calculatePressureForce(Sphere* defaultSp, int idx) {
	glm::vec4 res = glm::vec4(0, 0, 0, 0), dir;
	float distance;
	std::vector<Sphere*> neighbours = grid.getNeighbours(glm::vec2(defaultSp->center.x, defaultSp->center.y), *defaultSp);
	for (int i = 0; i < neighbours.size(); ++i) {
		if (i == idx) continue;
		distance = glm::length(defaultSp->center - particles[i]->center)/* - defaultSp->radius - particles[i]->radius*/;
		if (distance == 0) dir = glm::vec4(glm::circularRand(1.0f), 0, 0); 
		else dir = (defaultSp->center - particles[i]->center) / distance;
		res -= (pressure[i] + pressure[idx]) / ( 2) * dir * smoothingKernelDerived(distance);
	}

	return res;
}

void SPH2::checkBounds(Sphere* sp) {
	float damping = 0.65;
	if (sp->center.x < bounds[0].x + sp->radius) {
		sp->center.x = bounds[0].x + sp->radius;
		sp->velocity.x *= -1 * damping;
	}
	else if (sp->center.x > bounds[1].x - sp->radius) {
		sp->center.x = bounds[1].x - sp->radius;
		sp->velocity.x *= -1 * damping;
	}
	if (sp->center.y < bounds[0].y + sp->radius) {
		sp->center.y = bounds[0].y + sp->radius;
		sp->velocity.y *= -1 * damping;
	}
	else if (sp->center.y > bounds[2].y - sp->radius) {
		sp->center.y = bounds[2].y - sp->radius;
		sp->velocity.y *= -1 * damping;
	}
}


void SPH2::step(float dt) {
	if(dt == 0)dt = 0.01f;
	calculateDensities();
	calculatePressure();
#pragma omp parallel for 
	for (int i = 0; i < particles.size(); ++i) {
		particles[i]->velocity += downVec * g * dt;
		glm::vec4 Fp = calculatePressureForce(particles[i], i);
		particles[i]->velocity += Fp / density[i] * dt;
	}
#pragma omp parallel for 
	for (int i = 0; i < particles.size(); ++i) {
		particles[i]->center += particles[i]->velocity * dt;
		checkBounds(particles[i]);
		grid.updateParticle(*particles[i]);
	}
}