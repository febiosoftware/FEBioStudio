#version 440

// input
layout(location = 0) in vec3 v_tex;

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
} glob;

// volume render settings
layout(std140, binding = 1) uniform settings {
    vec4 col1;
    vec4 col2;
    vec4 col3;
    float Imin;
    float Imax;
    float Iscl;
    float IsclMin;
    float Amin;
    float Amax;
    float gamma;
    int cmap;
} vr;

// mesh-specific block
layout(std140, binding = 2) uniform MeshBlock {
    mat4 mv;
    vec4 col;
} mesh;

// texture sampler
layout(binding = 3) uniform sampler3D smp;

void main()
{
    // scale and clamp
    vec4 t = (texture(smp, v_tex) - vr.IsclMin)*vr.Iscl;
    t = (t - vr.Imin) / (vr.Imax - vr.Imin);
    t = clamp(t, 0, 1);

    // the "alpha" value is taken as the largest of xyz
    float f = t.x;
    if (t.y > f) f = t.y;
    if (t.z > f) f = t.z;
    
    // discard if transparent
    if (f <= 0.0) discard;

    // apply gamma correction
    if (vr.gamma != 1.0) f = pow(f, vr.gamma);

    // calculate user-scaled alpha
    float a = vr.Amin + f*(vr.Amax - vr.Amin);

    // colorize voxel
    vec3 c1 = vec3(t.x*vr.col1.x, t.x*vr.col1.y, t.x*vr.col1.z);
    vec3 c2 = vec3(t.y*vr.col2.x, t.y*vr.col2.y, t.y*vr.col2.z);
    vec3 c3 = vec3(t.z*vr.col3.x, t.z*vr.col3.y, t.z*vr.col3.z);
    vec3 c4 = c1 + c2 + c3;

    vec4 c = vec4(c4, a);
    fragColor = mesh.col*c;
}
