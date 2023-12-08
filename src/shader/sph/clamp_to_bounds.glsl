vec3 clamp_to_bounds(vec3 position) {
    if (position.x > G.high_bound.x) {
        position.x = G.high_bound.x;
    }
    if (position.y > G.high_bound.y) {
        position.y = G.high_bound.y;
    }
    if (position.z > G.high_bound.z) {
        position.z = G.high_bound.z;
    }
    if (position.x < G.low_bound.x) {
        position.x = G.low_bound.x;
    }
    if (position.y < G.low_bound.y) {
        position.y = G.low_bound.y;
    }
    if (position.z < G.low_bound.z) {
        position.z = G.low_bound.z;
    }
    return position;
}
