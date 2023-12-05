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
