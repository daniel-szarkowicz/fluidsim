struct Particle {
    vec3 position;
    float mass;

    vec3 predicted_position;
    float density;

    vec3 velocity;
    uint cell_key;

    ivec3 cell_pos;
    uint id;

    uint index_in_key;
    float pressure;
    float _padding1;
    float _padding2;
};
