#version 440

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec3 tex;

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec3 v_color;
layout(location = 3) out vec3 v_tex;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    vec4 lightPos;
    vec4 specColor;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mvp;
    mat4 mv;
    vec4 col;
    float specExp;
    float specStrength;
    float opacity;
    float useTexture;
    float useStipple;
} mesh;

// texture sampler
layout(binding = 2) uniform sampler2D smp;

void main()
{
    v_color = mesh.col.xyz; //color;
    v_normal = normalize((mesh.mv*vec4(normal, 0)).xyz);
    v_pos = (mesh.mv*vec4(position,1)).xyz;
    v_tex = tex;
    gl_Position = mesh.mvp * vec4(position, 1);
}
