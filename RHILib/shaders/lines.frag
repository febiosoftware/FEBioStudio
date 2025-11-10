#version 440

// input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec4 v_col;

// output
layout(location = 0) out vec4 fragColor;

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
    vec4 col;
    int useClipping;
    int useVertexColor;
} mesh;

void main()
{
    // clip-plane
    if (mesh.useClipping > 0)
    {
        float d = glob.clipPlane[0]*v_pos.x + glob.clipPlane[1]*v_pos.y + glob.clipPlane[2]*v_pos.z + glob.clipPlane[3];
        if (d < 0) discard;
    }

    vec4 col = mesh.col;
    if (mesh.useVertexColor > 0) col = v_col;

    fragColor = col;
}
