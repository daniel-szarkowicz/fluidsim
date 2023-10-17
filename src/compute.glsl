#version 430

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

struct Sphere {
    vec4 center;
    vec4 velocity;
    vec4 color;
    float radius;
};

layout(std430, binding = 3) readonly buffer inputs {
    Sphere spheres_in[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Sphere spheres_out[];
};

uniform vec4 gravity;
uniform vec3 low_bound;
uniform vec3 high_bound;
uniform float dt;

void main() {
    uint i = gl_GlobalInvocationID.x;
    Sphere s = spheres_in[i];
    s.velocity += gravity * dt;
    s.center += s.velocity * dt;
    bool collided = false;
    if (s.center.x > high_bound.x - s.radius) {
        s.center.x = high_bound.x - s.radius;
        s.velocity.x = -abs(s.velocity.x);
        collided = true;
    }
    if (s.center.y > high_bound.y - s.radius) {
        s.center.y = high_bound.y - s.radius;
        s.velocity.y = -abs(s.velocity.y);
        collided = true;
    }
    if (s.center.z > high_bound.z - s.radius) {
        s.center.z = high_bound.z - s.radius;
        s.velocity.z = -abs(s.velocity.z);
        collided = true;
    }
    if (s.center.x < low_bound.x + s.radius) {
        s.center.x = low_bound.x + s.radius;
        s.velocity.x = +abs(s.velocity.x);
        collided = true;
    }
    if (s.center.y < low_bound.y + s.radius) {
        s.center.y = low_bound.y + s.radius;
        s.velocity.y = +abs(s.velocity.y);
        collided = true;
    }
    if (s.center.z < low_bound.z + s.radius) {
        s.center.z = low_bound.z + s.radius;
        s.velocity.z = +abs(s.velocity.z);
        collided = true;
    }
    if (collided) {
        s.velocity *= 0.95;
    }
    spheres_out[i] = s;
}
