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
    uint key_count;

    uint selected_index;
    float _padding3;
    float _padding4;
    float _padding5;
};
