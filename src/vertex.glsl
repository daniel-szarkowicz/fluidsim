#version 400

uniform mat4 model_view;

in vec3 center;
in vec3 color;
in float radius;

out vec3 vColor;
out float vRadius;

void main() {
    gl_Position = model_view * vec4(center, 1.0);
    vColor = color;
    vRadius = radius;
}
