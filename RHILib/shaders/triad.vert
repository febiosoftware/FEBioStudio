#version 440

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

// output
layout(location = 0) out vec3 v_normal;
layout(location = 1) out vec3 v_color;

// global (shared) block
layout(std140, binding = 0) uniform Globals {
    mat4 projectionMatrix;
    vec4 lightPos;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mv;
} mesh;

// texture sampler
layout(binding = 2) uniform sampler2D smp;

void main()
{
    v_color = color;
    v_normal = normalize((mesh.mv*vec4(normal, 0)).xyz);
    gl_Position = glob.projectionMatrix*(mesh.mv * vec4(position, 1));
}
