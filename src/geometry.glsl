#version 400

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 projection;

in vec3 vColor[];
in float vRadius[];

out vec3 fColor;
out vec2 fPosition;

void main() {
    const vec2 positions[4] = vec2[4] (
        vec2(-1, -1),
        vec2(-1,  1),
        vec2( 1, -1),
        vec2( 1,  1)
    );

    fColor = vColor[0];
    for (int i = 0; i < 4; ++i) {
        fPosition = positions[i];
        gl_Position = projection *
            (gl_in[0].gl_Position + vec4(vRadius[0] * positions[i], 0, 0));
        EmitVertex();
    }
    EndPrimitive();
}
