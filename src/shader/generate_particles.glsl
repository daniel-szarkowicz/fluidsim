// uses Particle from particle.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Particle p[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Particle po[];
};

uniform uint prev_object_count;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }

    if (i < prev_object_count) {
        po[i] = p[i];
        return;
    }

    // 2D
    po[i].position = vec4(
        float(hash(i+0)) / float(uint(-1)),
        float(hash(i+1)) / float(uint(-1)),
        0,
        1
    ) * vec4(G.high_bound - G.low_bound, 1) + vec4(G.low_bound.xy, 0, 0);

    // // 3D
    // po[i].position = vec4(
    //     float(hash(i+0)) / float(uint(-1)),
    //     float(hash(i+1)) / float(uint(-1)),
    //     float(hash(i+2)) / float(uint(-1)),
    //     1
    // ) * vec4(G.high_bound - G.low_bound, 1) + vec4(G.low_bound, 0);

    po[i].velocity = vec4(0, 0, 0, 0);
    po[i].mass = 1;
}
