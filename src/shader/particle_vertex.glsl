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
    float relative_density = p[i].density - G.target_density;
    if (relative_density < 0) {
        vColor = mix(vec4(1, 1, 1, 1), vec4(0, 1, 0, 1), log(1-relative_density));
    } else {
        vColor = mix(vec4(1, 1, 1, 1), vec4(1, 0, 0, 1), log(1+relative_density));
    }
    vRadius = G.particle_size;
}
