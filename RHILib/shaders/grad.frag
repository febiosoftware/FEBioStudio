#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform GradientParams {
    vec4 colorTop;
    vec4 colorBottom;
    int orient;
} u;

void main()
{
    vec3 color;
    if (u.orient == 0)
        color = mix(u.colorBottom.xyz, u.colorTop.xyz, vUV.y);
    else
        color = mix(u.colorBottom.xyz, u.colorTop.xyz, vUV.x);
    fragColor = vec4(color, 1.0);
}
