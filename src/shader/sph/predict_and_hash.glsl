// uses Particle from particle.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Particle p[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Particle po[];
};

layout(std430, binding = 2) buffer atomics {
    uint key_counters[];
};

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    Particle particle = p[i];
    particle.predicted_position =
        particle.position + particle.velocity * G.delta_time;
    ivec4 cell_pos = cell_pos(particle.predicted_position);
    particle.cell_key = cell_key(cell_pos);
    particle.index_in_key = atomicAdd(key_counters[particle.cell_key + 1], 1);
    po[i] = particle;
}
