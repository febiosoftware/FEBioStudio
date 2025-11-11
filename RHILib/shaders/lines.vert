#version 440

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec4 v_col;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    vec4 lightPos;
    vec4 ambient;
    vec4 diffuse;
    vec4 specColor;
    vec4 clipPlane;
    int lightEnabled;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mvp;
    mat4 mv;
    vec4 col;
    int useClipping;
    int useVertexColor;
} mesh;

void main()
{
    vec4 r = mesh.mvp * vec4(position, 1);
    r.z -= 1.0e-4;
    v_pos = (mesh.mv*vec4(position,1)).xyz;
    v_col = color;
    gl_Position = r;
}
