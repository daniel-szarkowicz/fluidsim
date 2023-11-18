struct Globals {
    vec4 gravity;

    vec3 low_bound;
    float _padding1;

    vec3 high_bound;
    float _padding2;

    uint object_count;
    float particle_size;
    float smoothing_radius;
    float delta_time;

    float target_density;
    float pressure_multiplier;
    float collision_multiplier;
    float _padding3;
};
