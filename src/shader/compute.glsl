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
uniform float collision_multiplier;
uniform uint object_count;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= object_count) {
        return;
    }
    Sphere s = spheres_in[i];
    bool collided = false;
    for (uint j = 0; j < object_count; ++j) {
        if (i != j) {
            Sphere other = spheres_in[j];
            vec4 dir = s.center - other.center;
            if (length(dir) < s.radius + other.radius) {
                dir = normalize(dir);
                s.velocity = s.velocity - dot(s.velocity - other.velocity, dir)
                    / dot(dir, dir) * dir;
                vec4 coll_center = vec4(
                    (s.center.xyz * other.radius + other.center.xyz * s.radius)
                    / (s.radius + other.radius), 1);
                s.center = coll_center + dir * (s.radius);
                collided = true;
            }
        }
    }
    s.velocity += gravity * dt;
    s.center += s.velocity * dt;
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
        s.velocity *= collision_multiplier;
    }
    spheres_out[i] = s;
}
