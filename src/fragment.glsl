#version 400

in vec3 fCenter;
in vec3 fColor;
in float fRadius;
in vec3 fPosition;

out vec4 frag_color;

void main() {
    if (distance(fCenter, fPosition) < fRadius) {
        frag_color = vec4(fColor, 1.0);
    } else {
        discard;
    }
}
