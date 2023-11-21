// uses Particle from particle.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Particle p[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Particle po[];
};

layout(std430, binding = 2) readonly buffer keys {
    uint key_map[];
};

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }

    Particle particle = p[i];
    po[key_map[particle.cell_key] + particle.index_in_key] = particle;
}
