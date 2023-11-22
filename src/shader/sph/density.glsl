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
    float density = 0;
    for_neighbor(p[i], neighbor, {
        float distance = distance(p[i].predicted_position, neighbor.predicted_position);
        density += neighbor.mass * kernel(distance);
    });
    Particle particle = p[i];
    particle.density = density;
    po[i] = particle;
}
