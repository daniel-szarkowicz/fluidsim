uniform mat4 view_projection;

in vec3 position;

void main() {
    vec4 pos = vec4(0, 0, 0, 1);
    if (position.x < 0) {
        pos.x = -position.x * G.low_bound.x;
    } else {
        pos.x = position.x * G.high_bound.x;
    }
    if (position.y < 0) {
        pos.y = -position.y * G.low_bound.y;
    } else {
        pos.y = position.y * G.high_bound.y;
    }
    if (position.z < 0) {
        pos.z = -position.z * G.low_bound.z;
    } else {
        pos.z = position.z * G.high_bound.z;
    }
    gl_Position = view_projection * pos;
}
