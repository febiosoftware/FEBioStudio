#version 440

// input
layout(location = 0) in vec3 v_normal;
layout(location = 1) in vec3 v_color;

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
} mesh;

void main()
{
    // do lighting calculation
    vec3 L = normalize(glob.lightPos.xyz);
    vec3 N = normalize(v_normal);

    vec3 f_col = vec3(0,0,0);

    vec3 col = v_color;

    // ambient value
    f_col += col*0.2;

    // front-lit
    float b = max(dot(N, vec3(0,0,1)),0);
    f_col += col*(b*0.2);

    // diffuse component
    float a = max(dot(N, L),0);
    f_col += col*a;

    // return final color
    fragColor = vec4(f_col, 1);
}
