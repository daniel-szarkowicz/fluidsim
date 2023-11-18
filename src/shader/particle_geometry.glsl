layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 projection;

in vec4 vColor[];
in float vRadius[];

out vec4 fColor;
out vec4 fPosition;

void main() {
    const vec4 offsets[4] = vec4[4] (
        vec4(-1, -1, 0, 0),
        vec4(-1,  1, 0, 0),
        vec4( 1, -1, 0, 0),
        vec4( 1,  1, 0, 0)
    );

    fColor = vColor[0];
    for (int i = 0; i < 4; ++i) {
        fPosition = offsets[i];
        gl_Position = projection *
            (gl_in[0].gl_Position + vRadius[0] * offsets[i]);
        EmitVertex();
    }
    EndPrimitive();
}
