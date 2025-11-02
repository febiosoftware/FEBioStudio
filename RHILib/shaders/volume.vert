#version 440

// input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 tex;

// output
layout(location = 0) out vec3 v_tex;

// volume render settings
layout(std140, binding = 0) uniform settings {
    vec4 col1;
    vec4 col2;
    vec4 col3;
    float Imin;
    float Imax;
    float Iscl;
    float IsclMin;
    float Amin;
    float Amax;
    float gamma;
    int cmap;
} ops;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mvp;
    vec4 col;
} mesh;

// texture sampler
layout(binding = 2) uniform sampler3D smp;

void main()
{
    v_tex = tex;
    gl_Position = mesh.mvp * vec4(pos, 1);
}
