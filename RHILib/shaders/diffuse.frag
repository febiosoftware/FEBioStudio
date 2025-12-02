#version 440

// input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec4 v_color;

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
    int lightEnabled;
    float pointSize;
    float zOffset;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mv;
    vec4 col;
    int useStipple;
    int useClipping;
    int useLighting;
    int useVertexColor;
} mesh;

void main()
{
    vec4 col = mesh.col;
    if (mesh.useVertexColor > 0)
        col = v_color;

    // check stippling
    if ((mesh.useStipple > 0) || (col.a < 0.95))
    {
        ivec2 p = ivec2(mod(gl_FragCoord.xy, 8.0)); // 8x8 pattern
        bool visible = ((p.x + p.y) % 2) == 0;  // checker pattern
        if (!visible)
            discard;
    }
    col.a = 1;

    // clip-plane
    if (mesh.useClipping > 0)
    {
        float d = glob.clipPlane[0]*v_pos.x + glob.clipPlane[1]*v_pos.y + glob.clipPlane[2]*v_pos.z + glob.clipPlane[3];
        if (d < 0) discard;
    }

    // do lighting calculation
    vec3 V = normalize(v_pos);
    vec3 L = normalize(glob.lightPos.xyz);
    vec3 N = normalize(v_normal);

    vec4 f_col = vec4(0,0,0,1);

    if (mesh.useLighting > 0)
    {
        // ambient value
        f_col += glob.ambient*col;

        // diffuse component
        float a = abs(dot(N, L));
        a = clamp(a, 0.0, 1.0);
        vec3 diffuseColor = glob.diffuse.xyz * col.xyz;
        f_col.xyz += diffuseColor*a;
    }
    else
    {
        f_col = col;
    }

    // return final color
    fragColor = f_col;
}
