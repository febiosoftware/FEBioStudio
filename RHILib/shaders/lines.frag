#version 440

// output
layout(location = 0) out vec4 fragColor;

// mesh-specific block
layout(std140, binding = 0) uniform MeshBlock {
    mat4 mvp;
    mat4 mv;
    vec4 col;
} mesh;

void main()
{
    fragColor = vec4(mesh.col.xyz, 0.25f);
}
