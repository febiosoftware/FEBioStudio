#version 440

// input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_col;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    mat4 projectionMatrix;
    vec4 lightPos;
    vec4 ambient;
    vec4 diffuse;
    vec4 specColor;
    vec4 clipPlane;
    int lightEnabled;
    float pointSize;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mv;
    vec4 col;
    int useClipping;
    int useVertexColor;
} mesh;

void main()
{
    vec4 r = glob.projectionMatrix*(mesh.mv * vec4(pos, 1));
    r.z -= 1.0e-4;
    v_pos = (mesh.mv*vec4(pos,1)).xyz;
    v_col = col;
    gl_Position = r;
    gl_PointSize = glob.pointSize;
}
