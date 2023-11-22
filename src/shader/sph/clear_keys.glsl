layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 2) writeonly buffer inputs {
    uint k[];
};

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.key_count + 1) {
        return;
    }
    k[i] = 0;
}
