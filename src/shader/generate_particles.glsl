uniform uint prev_object_count;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= G.object_count) {
        return;
    }

    if (i < prev_object_count) {
        po[i] = p[i];
        po[i].id = i;
        return;
    }

    po[i].position = vec3(
        float(hash(i+0)) / float(uint(-1)),
        float(hash(i+1)) / float(uint(-1)),
        float(hash(i+2)) / float(uint(-1))
    ) * vec3(G.high_bound - G.low_bound) + vec3(G.low_bound);

    po[i].velocity = vec3(0, 0, 0);
    po[i].mass = 1;
    po[i].id = i;
}
