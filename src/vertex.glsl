#version 400

in vec3 center;
in vec3 color;
in float radius;

out vec3 vColor;
out float vRadius;

void main() {
    gl_Position = vec4(center, 1.0);
    vColor = color;
    vRadius = radius;
}
