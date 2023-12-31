const ivec3 cell_neighbors[27] = ivec3[] (
    ivec3( 1,  1,  1),
    ivec3( 1,  1,  0),
    ivec3( 1,  1, -1),
    ivec3( 1,  0,  1),
    ivec3( 1,  0,  0),
    ivec3( 1,  0, -1),
    ivec3( 1, -1,  1),
    ivec3( 1, -1,  0),
    ivec3( 1, -1, -1),

    ivec3( 0,  1,  1),
    ivec3( 0,  1,  0),
    ivec3( 0,  1, -1),
    ivec3( 0,  0,  1),
    ivec3( 0,  0,  0),
    ivec3( 0,  0, -1),
    ivec3( 0, -1,  1),
    ivec3( 0, -1,  0),
    ivec3( 0, -1, -1),

    ivec3(-1,  1,  1),
    ivec3(-1,  1,  0),
    ivec3(-1,  1, -1),
    ivec3(-1,  0,  1),
    ivec3(-1,  0,  0),
    ivec3(-1,  0, -1),
    ivec3(-1, -1,  1),
    ivec3(-1, -1,  0),
    ivec3(-1, -1, -1)
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

ivec3 cell_pos(vec3 pos) {
    return ivec3(floor(pos / G.smoothing_radius));
}

uint cell_key(ivec3 cell_pos) {
    ivec3 cell_pos_offset = cell_pos - G.low_bound_cell;
    uint cell_hash = 0;
    cell_hash = cell_hash * G.grid_size.x + cell_pos_offset.x % G.grid_size.x;
    cell_hash = cell_hash * G.grid_size.y + cell_pos_offset.y % G.grid_size.y;
    cell_hash = cell_hash * G.grid_size.z + cell_pos_offset.z % G.grid_size.z;
    return cell_hash % G.key_count;
}
