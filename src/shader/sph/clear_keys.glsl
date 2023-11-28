void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.key_count + 1) {
        return;
    }
    k[i] = 0;
}
