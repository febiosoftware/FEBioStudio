#version 440

// input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_col;
layout(location = 2) in vec2 v_corner;

// output
layout(location = 0) out vec4 fragColor;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    mat4 projectionMatrix;
    vec4 lightPos;
    vec4 ambient;
    vec4 diffuse;
    vec4 specColor;
    vec4 clipPlane;
    vec2 viewport;  // viewport size in pixels
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
    float pointSize;
    int useDepthBias;
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
    if (mesh.useVertexColor > 0)
        col = vec4(v_col, 1.f);

    float d = length(v_corner) - 1.0;
    float width = fwidth(d);
    float alpha = 1.0 - smoothstep(-width, width, d);
    if (alpha <= 0.0)
        discard;

    fragColor = vec4(col.rgb, col.a * alpha);
}
