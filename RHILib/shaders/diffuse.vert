#version 440

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec4 v_color;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    mat4 projectionMatrix;
    vec4 lightPos;
    vec4 ambient;
    vec4 diffuse;
    vec4 specColor;
    vec4 clipPlane;
    int lightEnabled;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mv;
    int useStipple;
    int useClipping;
    int useLighting;
} mesh;

void main()
{
    v_pos = (mesh.mv*vec4(position,1)).xyz;
    v_normal = normalize((mesh.mv*vec4(normal, 0)).xyz);
    v_color = color;
    gl_Position = glob.projectionMatrix*(mesh.mv * vec4(position, 1));
}
