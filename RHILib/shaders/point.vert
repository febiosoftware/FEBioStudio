#version 440

// input
layout(location = 0) in vec3 position;

// output
layout(location = 0) out vec3 v_pos;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    vec4 lightPos;
    vec4 specColor;
    vec4 clipPlane;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mvp;
    mat4 mv;
    vec4 col;
    int useClipping;
} mesh;

void main()
{
    vec4 r = mesh.mvp * vec4(position, 1);
    r.z -= 1.0e-4;
    v_pos = (mesh.mv*vec4(position,1)).xyz;
    gl_Position = r;
    gl_PointSize = 3.f;
}
