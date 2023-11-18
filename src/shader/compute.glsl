// uses Particle from particle.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Particle p[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Particle po[];
};

uniform vec4 gravity;
uniform vec3 low_bound;
uniform vec3 high_bound;
uniform float dt;
uniform float collision_multiplier;
uniform uint object_count;

uniform float particle_size = 0.1;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= object_count) {
        return;
    }
    Particle s = p[i];
    bool collided = false;
    for (uint j = 0; j < object_count; ++j) {
        if (i != j) {
            Particle other = p[j];
            vec4 dir = s.position - other.position;
            if (length(dir) < 2 * particle_size) {
                dir = normalize(dir);
                s.velocity = s.velocity - dot(s.velocity - other.velocity, dir)
                    / dot(dir, dir) * dir;
                vec4 coll_center = vec4(
                    (s.position.xyz * particle_size
                        + other.position.xyz * particle_size)
                    / (2 * particle_size), 1);
                s.position = coll_center + dir * (particle_size);
                collided = true;
            }
        }
    }
    s.velocity += gravity * dt;
    s.position += s.velocity * dt;
    if (s.position.x > high_bound.x - particle_size) {
        s.position.x = high_bound.x - particle_size;
        s.velocity.x = -abs(s.velocity.x);
        collided = true;
    }
    if (s.position.y > high_bound.y - particle_size) {
        s.position.y = high_bound.y - particle_size;
        s.velocity.y = -abs(s.velocity.y);
        collided = true;
    }
    if (s.position.z > high_bound.z - particle_size) {
        s.position.z = high_bound.z - particle_size;
        s.velocity.z = -abs(s.velocity.z);
        collided = true;
    }
    if (s.position.x < low_bound.x + particle_size) {
        s.position.x = low_bound.x + particle_size;
        s.velocity.x = +abs(s.velocity.x);
        collided = true;
    }
    if (s.position.y < low_bound.y + particle_size) {
        s.position.y = low_bound.y + particle_size;
        s.velocity.y = +abs(s.velocity.y);
        collided = true;
    }
    if (s.position.z < low_bound.z + particle_size) {
        s.position.z = low_bound.z + particle_size;
        s.velocity.z = +abs(s.velocity.z);
        collided = true;
    }
    if (collided) {
        s.velocity *= collision_multiplier;
    }
    po[i] = s;
}
