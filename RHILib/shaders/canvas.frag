#version 440

// input
layout(location = 0) in vec2 v_uv;

// output
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform OverlayUBO {
    vec2 viewport;  // (W, H)
} ub;

layout(binding = 1) uniform sampler2D tex;

void main()
{
    vec4 c = texture(tex, v_uv);
    fragColor = vec4(c.rgb * c.a, c.a);
}
