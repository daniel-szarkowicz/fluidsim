layout(std430, binding = 9) readonly buffer box_ssbo {
    vec4 box[24];
};

uniform mat4 view_projection;

void main() {
    uint i = gl_InstanceID * 2 + gl_VertexID;
    vec3 position = box[i].xyz;
    vec4 pos = vec4(0, 0, 0, 1);
    if (position.x < 0) {
        pos.x = -position.x * G.low_bound.x - G.particle_size;
    } else {
        pos.x = position.x * G.high_bound.x + G.particle_size;
    }
    if (position.y < 0) {
        pos.y = -position.y * G.low_bound.y - G.particle_size;
    } else {
        pos.y = position.y * G.high_bound.y + G.particle_size;
    }
    if (position.z < 0) {
        pos.z = -position.z * G.low_bound.z - G.particle_size;
    } else {
        pos.z = position.z * G.high_bound.z + G.particle_size;
    }
    gl_Position = view_projection * pos;
}
