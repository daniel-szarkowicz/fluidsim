#version 400
uniform float radius;

in vec3 fColor;
in vec3 fCenter;
in vec3 fPosition;
out vec4 frag_color;

void main() {
    if (distance(fCenter, fPosition) < radius) {
        frag_color = vec4(fColor, 1.0);
    } else {
        frag_color = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
