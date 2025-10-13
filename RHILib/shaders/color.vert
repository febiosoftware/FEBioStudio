#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_color;
layout(location = 2) out vec3 v_normal;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
    mat4 mv;
};

void main()
{
    v_color = color;
    v_normal = (mv*vec4(normal, 0)).xyz;
    v_pos = (mv*position).xyz;
    gl_Position = mvp * position;
}
