#version 440

// input
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec3 v_color;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    vec4 lightPos;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mvp;
    mat4 mv;
} mesh;

void main()
{
    v_color = color;
    v_normal = (mesh.mv*vec4(normal, 0)).xyz;
    v_pos = (mesh.mv*position).xyz;
    gl_Position = mesh.mvp * position;
}
