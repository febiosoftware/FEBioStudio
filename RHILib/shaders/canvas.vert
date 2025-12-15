#version 440

// input
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;

// output
layout(location = 0) out vec2 v_uv;

layout(std140, binding = 0) uniform OverlayUBO {
    vec2 viewport;  // (W, H)
} ub;

layout(binding = 1) uniform sampler2D tex;

void main()
{
    v_uv =  uv;

    vec2 ndc;
    ndc.x =  (2.0 * pos.x / ub.viewport.x) - 1.0;
    ndc.y = -(2.0 * pos.y / ub.viewport.y) + 1.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
}
