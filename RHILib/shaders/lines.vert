#version 440

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 texCoord;
layout(location = 2) in vec4 color;

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_tex;
layout(location = 2) out vec4 v_col;

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
    float zOffset;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mv;
    vec4 col;
    int useClipping;
    int useVertexColor;
    int useTexture;
} mesh;

void main()
{
    vec4 r = glob.projectionMatrix*(mesh.mv * vec4(position, 1));
    r.z -= glob.zOffset;
    v_pos = (mesh.mv*vec4(position,1)).xyz;
    v_col = color;
    v_tex = texCoord;
    gl_Position = r;
}
