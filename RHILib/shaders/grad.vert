#version 440

layout(location = 0) out vec2 vUV;

layout(std140, binding = 0) uniform GradientParams {
    vec4 colorTop;
    vec4 colorBottom;
    int orient;
} u;

void main()
{
    // Fullscreen triangle coordinates (no vertex buffer)
    const vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

    // Map from [-1, 1] clip space to [0, 1] texture space
    vUV = (positions[gl_VertexIndex] * 0.5) + 0.5;
}
