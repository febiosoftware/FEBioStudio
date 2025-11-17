#version 440

// input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_tex;
layout(location = 3) in vec4 v_color;

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

// mesh-specific block
layout(std140, binding = 1) uniform MeshBlock {
    mat4 mv;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float specExp;
    float opacity;
    float reflection;
    int useTexture;
    int useStipple;
    int useClipping;
    int useVertexColor;
    int useLighting;
    int frontOnly;
    int useFrontLight;
} mesh;

// texture sampler
layout(binding = 2) uniform sampler2D smp;
layout(binding = 3) uniform sampler2D envSmp;

void main()
{
    // check stippling
    if (mesh.useStipple > 0)
    {
        ivec2 p = ivec2(mod(gl_FragCoord.xy, 8.0)); // 8x8 pattern
        bool visible = ((p.x + p.y) % 2) == 0;  // checker pattern
        if (!visible)
            discard;
    }

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

    vec4 col = v_color;

    // textured rendering
    if (mesh.useTexture > 0)
    {
        col.xyz *= texture(smp, v_tex.xy).xyz;

        if ((mesh.useLighting > 0) && (glob.lightEnabled > 0))
        {
            // diffuse component
            float a = max(dot(N, L),0);
            a = clamp(a, 0.3, 1); // avoid too dark colors
            vec3 diffuseColor = glob.diffuse.xyz * col.xyz;
            col.xyz = diffuseColor*a;
        }

        f_col = col;
    }
    else if ((mesh.useLighting > 0) && (glob.lightEnabled > 0))
    {
        if (mesh.reflection > 0)
        {
            float r = clamp(mesh.reflection, 0, 1);
            const float PI = 3.14159265358979323846;

            vec3 R = reflect(V, N);
            float u = atan(R.z, R.x) / (2.0 * PI) + 0.5;
            float v = 0.5 - asin(R.y) / PI;
            vec4 envCol = texture(envSmp, vec2(u,v));
            col.xyz = envCol.xyz*r + col.xyz*(1-r);
        }

        // ambient value
        f_col += glob.ambient*mesh.ambient;

        if (gl_FrontFacing || (mesh.frontOnly == 0)) {

            if (!gl_FrontFacing)
                N = -N;

            // front-lit
            if (mesh.useFrontLight > 0) {
                float b = max(dot(N, vec3(0,0,1)),0);
                f_col += col*(b*0.2);
            }

            // diffuse component
            float a = max(dot(N, L),0);
            if (a > 0)
            {
                vec3 diffuseColor = glob.diffuse.xyz * col.xyz;
                f_col.xyz += diffuseColor*a;
            }

            // specular component
            if (mesh.specExp > 0) {
                vec3 R = normalize(reflect(V, N));
                float c = clamp(dot(R, L), 1e-4, 0.99999);
                float se = clamp(128.0 * mesh.specExp, 0.0, 128.0);
                float s = pow(c, se);
                s = clamp(s, 0, 1);

                vec3 specColor = glob.specColor.xyz * mesh.specular.xyz;
                f_col.xyz += specColor*s;
            }
        }
        else {
            // only diffuse for backfacing triangles
            float a = max(dot(-N, L),0);
            f_col.xyz += vec3(1, 0.7, 0.7)*a;
        }
    }
    else 
    {
        f_col = col;
    }

    // return final color
    fragColor = f_col;
}
