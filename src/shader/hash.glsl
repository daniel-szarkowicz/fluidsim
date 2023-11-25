const ivec4 cell_neighbors[27] = ivec4[] (
    ivec4( 1,  1,  1, 0),
    ivec4( 1,  1,  0, 0),
    ivec4( 1,  1, -1, 0),
    ivec4( 1,  0,  1, 0),
    ivec4( 1,  0,  0, 0),
    ivec4( 1,  0, -1, 0),
    ivec4( 1, -1,  1, 0),
    ivec4( 1, -1,  0, 0),
    ivec4( 1, -1, -1, 0),

    ivec4( 0,  1,  1, 0),
    ivec4( 0,  1,  0, 0),
    ivec4( 0,  1, -1, 0),
    ivec4( 0,  0,  1, 0),
    ivec4( 0,  0,  0, 0),
    ivec4( 0,  0, -1, 0),
    ivec4( 0, -1,  1, 0),
    ivec4( 0, -1,  0, 0),
    ivec4( 0, -1, -1, 0),

    ivec4(-1,  1,  1, 0),
    ivec4(-1,  1,  0, 0),
    ivec4(-1,  1, -1, 0),
    ivec4(-1,  0,  1, 0),
    ivec4(-1,  0,  0, 0),
    ivec4(-1,  0, -1, 0),
    ivec4(-1, -1,  1, 0),
    ivec4(-1, -1,  0, 0),
    ivec4(-1, -1, -1, 0)
);

uint hash(uint x) {
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

uint cell_hash(ivec4 cell_pos) {
    uint cell_hash = 0;
    cell_hash = hash(cell_hash + uint(cell_pos.x));
    cell_hash = hash(cell_hash + uint(cell_pos.y));
    cell_hash = hash(cell_hash + uint(cell_pos.z));
    return cell_hash;
}

uint cell_key(uint cell_hash) {
    return cell_hash % G.key_count;
}

ivec4 cell_pos(vec4 pos) {
    return ivec4(floor(pos / G.smoothing_radius));
}
