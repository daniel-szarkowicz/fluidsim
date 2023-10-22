#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

struct Particle2 {
	glm::vec2 pos, vel;
	float mass, density, radius;

	Particle2(glm::vec2 p = glm::vec2(0,0), glm::vec2 vel = glm::vec2(0, 0), float m = 0.0f, float d = 0.0f, float r = 0.0f) : pos(p), vel(vel), mass(m), density(d), radius(r){}

	Particle2(const Particle2& p) {
		pos = p.pos;
		vel = p.vel;
		mass = p.mass;
		density = p.density;
		radius = p.radius;
	}
};

struct Sphere {
	glm::vec4 center;         // 128 bits
	glm::vec4 velocity;       // 128 bits
	glm::vec4 color;          // 128 bits
	GLfloat density;	 // 32 bits
	GLfloat mass;        // 32 bits
	GLfloat pressure;    // 32 bits
	GLfloat radius;      // 32 bits
};
