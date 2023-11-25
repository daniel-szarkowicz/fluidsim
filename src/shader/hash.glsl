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

ivec4 cell_pos(vec4 pos) {
    return ivec4(floor(pos / G.smoothing_radius));
}

uint cell_key(ivec4 cell_pos_) {
    // TODO: move to globals
    ivec4 low_cell_pos = cell_pos(vec4(G.low_bound, 1));
    ivec4 high_cell_pos = cell_pos(vec4(G.high_bound, 1));
    ivec4 grid_size = high_cell_pos - low_cell_pos + ivec4(1, 1, 1, 0);

    ivec4 cell_upos = cell_pos_ - low_cell_pos;
    uint cell_hash = 0;
    cell_hash = cell_hash * grid_size.x + cell_upos.x % grid_size.x;
    cell_hash = cell_hash * grid_size.y + cell_upos.y % grid_size.y;
    cell_hash = cell_hash * grid_size.z + cell_upos.z % grid_size.z;
    return cell_hash % G.key_count;
}
