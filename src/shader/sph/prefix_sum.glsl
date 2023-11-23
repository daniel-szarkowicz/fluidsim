layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 2) readonly buffer inputs {
    uint k[];
};

layout(std430, binding = 5) writeonly buffer outputs {
    uint ko[];
};

uniform uint offset;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.key_count + 1) {
        return;
    }
    if (i < offset) {
        ko[i] = k[i];
    } else {
        ko[i] = k[i - offset] + k[i];
    }
}
