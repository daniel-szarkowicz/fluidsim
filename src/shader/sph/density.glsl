// uses Particle from particle.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Particle p[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Particle po[];
};

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }
    float density = 0;
    for (uint j = 0; j < G.object_count; ++j) {
        float distance = distance(p[i].position, p[j].position);
        density += p[j].mass * kernel(distance);
    }
    po[i].density = density;
}
