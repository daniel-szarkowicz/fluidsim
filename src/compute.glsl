#version 430

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Sphere {
    vec4 center;
    vec4 velocity;
    vec4 color;
    float radius;
};

layout(std430, binding = 3) buffer points {
    Sphere spheres[];
};

uniform vec4 gravity;
uniform vec3 low_bound;
uniform vec3 high_bound;

void main() {
    uint i = gl_GlobalInvocationID.x;
    Sphere s = spheres[i];
    s.velocity += gravity;
    s.center += s.velocity;
    if (s.center.x > high_bound.x - s.radius) {
        s.center.x = high_bound.x - s.radius;
        s.velocity.x = -abs(s.velocity.x);
    }
    if (s.center.y > high_bound.y - s.radius) {
        s.center.y = high_bound.y - s.radius;
        s.velocity.y = -abs(s.velocity.y);
    }
    if (s.center.z > high_bound.z - s.radius) {
        s.center.z = high_bound.z - s.radius;
        s.velocity.z = -abs(s.velocity.z);
    }
    if (s.center.x < low_bound.x + s.radius) {
        s.center.x = low_bound.x + s.radius;
        s.velocity.x = +abs(s.velocity.x);
    }
    if (s.center.y < low_bound.y + s.radius) {
        s.center.y = low_bound.y + s.radius;
        s.velocity.y = +abs(s.velocity.y);
    }
    if (s.center.z < low_bound.z + s.radius) {
        s.center.z = low_bound.z + s.radius;
        s.velocity.z = +abs(s.velocity.z);
    }
    spheres[i] = s;
}
