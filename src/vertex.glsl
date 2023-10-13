#version 400
in vec3 pos;
in vec3 color;

out vec3 vColor;

void main() {
    gl_Position = vec4(pos, 1.0);
    vColor = color;
}
