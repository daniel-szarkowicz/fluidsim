#define for_neighbor_unchecked(particle, neighbor, block)                      \
do {                                                                           \
    ivec4 _cell_pos_ = particle.cell_pos;                                      \
    for (uint _ci_ = 0; _ci_ < 27; ++_ci_) {                                   \
        ivec4 _curr_cell_pos_ = _cell_pos_ + cell_neighbors[_ci_];             \
        if (!any(lessThan(_curr_cell_pos_, G.low_bound_cell))                  \
            && !any(greaterThan(_curr_cell_pos_, G.high_bound_cell))           \
        ) {                                                                    \
            uint _key_ = cell_key(_curr_cell_pos_);                            \
            for (uint _i_ = key_map[_key_]; _i_ < key_map[_key_ + 1]; ++_i_) { \
                Particle neighbor = p[_i_];                                    \
                {block}                                                        \
            }                                                                  \
        }                                                                      \
    }                                                                          \
} while(false);

#define for_neighbor(particle, neighbor, block)                                \
for_neighbor_unchecked(particle, neighbor, {                                   \
    if (neighbor.cell_pos == _curr_cell_pos_ &&                                \
        distance(particle.predicted_position, neighbor.predicted_position)     \
            <= G.smoothing_radius                                              \
    ) {block}                                                                  \
});

