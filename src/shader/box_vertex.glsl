uniform mat4 view_projection;
uniform vec3 low_bound;
uniform vec3 high_bound;

in vec3 position;

void main() {
    vec4 pos = vec4(0, 0, 0, 1);
    if (position.x < 0) {
        pos.x = -position.x * low_bound.x;
    } else {
        pos.x = position.x * high_bound.x;
    }
    if (position.y < 0) {
        pos.y = -position.y * low_bound.y;
    } else {
        pos.y = position.y * high_bound.y;
    }
    if (position.z < 0) {
        pos.z = -position.z * low_bound.z;
    } else {
        pos.z = position.z * high_bound.z;
    }
    gl_Position = view_projection * pos;
}
