#version 440

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
} mesh;

void main()
{
    vec3 eye   = normalize(v_pos);
    vec3 light = normalize(glob.lightPos.xyz);

    vec3 f_col = vec3(0,0,0);

    // ambient value
    f_col += v_color*0.2;

    if (gl_FrontFacing) {

        // front-lit
        float b = max(dot(v_normal, vec3(0,0,1)),0);
        f_col += v_color*(b*0.2);

        // diffuse component
        float a = max(dot(v_normal, light),0);
        f_col += v_color*a;

        // specular component
        vec3 r = normalize(eye - v_normal*(2.0*dot(v_normal,eye)));
        float c = dot(r,light);
        if (c > 0) {
            float s = pow(c, 64);
            f_col += vec3(0.8,0.8,0.8)*s;   
        }
    }
    else {
        // only diffuse for backfacing triangles
        float a = max(dot(-v_normal, light),0);
        f_col += vec3(1, 0.7, 0.7)*a;
    }

    // return final color
    fragColor = vec4(f_col, 1);
}
