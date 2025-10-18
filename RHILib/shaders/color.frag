#version 440
precision highp float;

// input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_color;

// output
layout(location = 0) out vec4 fragColor;

// global (shared) block
layout(std140, binding = 0) uniform GlobalBlock {
    vec4 lightPos;
} glob;

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mvp;
    mat4 mv;
    vec4 col;
    float spec;
} mesh;

void main()
{
    vec3 V = normalize(v_pos);
    vec3 L = normalize(glob.lightPos.xyz);
    vec3 N = normalize(v_normal);

    vec3 f_col = vec3(0,0,0);

    // ambient value
    f_col += v_color*0.2;

    if (gl_FrontFacing) {

        // front-lit
        float b = max(dot(N, vec3(0,0,1)),0);
        f_col += v_color*(b*0.2);

        // diffuse component
        float a = max(dot(N, L),0);
        f_col += v_color*a;

        // specular component
        vec3 R = normalize(reflect(V, N));
        float c = clamp(dot(R, L), 1e-4, 0.99999);
        float se = clamp(64.0 * mesh.spec, 0.0, 64.0);
        float s = pow(c, se);
        s = clamp(s, 0, 1);
        f_col += vec3(0.8)*s;   
    }
    else {
        // only diffuse for backfacing triangles
        float a = max(dot(-N, L),0);
        f_col += vec3(1, 0.7, 0.7)*a;
    }

    // return final color
    fragColor = vec4(f_col, 1);
}
