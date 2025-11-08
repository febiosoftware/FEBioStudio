#version 440

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tex;
layout(location = 3) in vec4 color;

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec3 v_tex;
layout(location = 3) out vec4 v_color;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    vec4 lightPos;
    vec4 ambient;
    vec4 specColor;
    vec4 clipPlane;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mvp;
    mat4 mv;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float specExp;
    float opacity;
    float reflection;
    int useTexture;
    int useStipple;
    int useClipping;
    int useVertexColor;
    int useLighting;
    int frontOnly;
} mesh;

// texture sampler
layout(binding = 2) uniform sampler2D smp;
layout(binding = 3) uniform sampler2D envSmp;

void main()
{
    float a = clamp(mesh.opacity, 0.0, 1.0);

    if (mesh.useVertexColor > 0) v_color = color;
    else {
        v_color = mesh.diffuse;
        v_color.a = mesh.opacity;
    }

    v_normal = normalize((mesh.mv*vec4(normal, 0)).xyz);
    v_pos = (mesh.mv*vec4(position,1)).xyz;
    v_tex = tex;
    gl_Position = mesh.mvp * vec4(position, 1);
}
