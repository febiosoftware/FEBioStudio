#version 440

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_color;
layout(location = 2) in vec3 v_normal;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
    mat4 Q;
};

void main()
{
    vec3 eye   = normalize(v_pos);
    vec3 light = normalize(vec3(0,0,1));

    vec3 f_col = vec3(0,0,0);

    // diffuse component
    float a = dot(v_normal, light);
    if (a < 0) {
        a = -a;
        f_col = vec3(1, 0.7, 0.7)*a;
    }
    else
        f_col = v_color*a;

    // specular component
    vec3 r = normalize(-eye + v_normal*(2.0*dot(v_normal,eye)));
    float c = pow(-dot(r,light),64);

    if (c > 0)
        f_col += vec3(0.5,0.4,0.4)*c;

    // return final color
    fragColor = vec4(f_col, 1);
}
