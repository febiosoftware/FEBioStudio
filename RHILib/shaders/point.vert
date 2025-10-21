#version 440

// input
layout(location = 0) in vec3 position;

// mesh-specific block
layout(std140, binding = 0) uniform MeshBlock {
    mat4 mvp;
    mat4 mv;
    vec4 col;
} mesh;

void main()
{
    vec4 r = mesh.mvp * vec4(position, 1);
    r.z -= 1.0e-4;
    gl_Position = r;
    gl_PointSize = 3.f;
}
