// uses particle from particle.glsl

layout(std430, binding = 3) readonly buffer points {
    Particle p[];
};

uniform mat4 view;

out vec4 vColor;
out float vRadius;

void main() {
    uint i = gl_InstanceID;
    gl_Position = view * p[i].position;
    // float speed = length(p[i].velocity)/10;
    // vColor = vec4(speed, speed, speed, 1);
    vColor = vec4(p[i].density.xxx/100, 1);
    vRadius = 0.1;
}
