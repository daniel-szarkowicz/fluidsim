// uses Particle from particle.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Particle p[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Particle po[];
};

float density_to_pressure(float density) {
    return (density - G.target_density) * G.pressure_multiplier;
}

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    Particle particle = p[i];
    particle.velocity += G.gravity * G.delta_time;
    particle.position += particle.velocity * G.delta_time;
    if (particle.position.x > G.high_bound.x - G.particle_size) {
        particle.position.x = G.high_bound.x - G.particle_size;
        particle.velocity.x = -abs(particle.velocity.x) * G.collision_multiplier;
    }
    if (particle.position.y > G.high_bound.y - G.particle_size) {
        particle.position.y = G.high_bound.y - G.particle_size;
        particle.velocity.y = -abs(particle.velocity.y) * G.collision_multiplier;
    }
    if (particle.position.z > G.high_bound.z - G.particle_size) {
        particle.position.z = G.high_bound.z - G.particle_size;
        particle.velocity.z = -abs(particle.velocity.z) * G.collision_multiplier;
    }
    if (particle.position.x < G.low_bound.x - G.particle_size) {
        particle.position.x = G.low_bound.x - G.particle_size;
        particle.velocity.x = abs(particle.velocity.x) * G.collision_multiplier;
    }
    if (particle.position.y < G.low_bound.y - G.particle_size) {
        particle.position.y = G.low_bound.y - G.particle_size;
        particle.velocity.y = abs(particle.velocity.y) * G.collision_multiplier;
    }
    if (particle.position.z < G.low_bound.z - G.particle_size) {
        particle.position.z = G.low_bound.z - G.particle_size;
        particle.velocity.z = abs(particle.velocity.z) * G.collision_multiplier;
    }
    particle.predicted_position =
        particle.position + particle.velocity * G.delta_time;
    po[i] = particle;
}
