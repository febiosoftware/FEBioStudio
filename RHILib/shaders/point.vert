#version 440

// input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_col;

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
    vec2 viewport;  // viewport size in pixels
    int useClipping;
    int useVertexColor;
    float pointSize;
    int useDepthBias;
} mesh;

// Large points are drawn as screen-aligned squares, but every pixel of the square has
// the depth of the point's center. Where the surface under the point is tilted relative
// to the screen, part of the square then ends up behind the surface and gets hidden.
// To prevent this, the point is moved toward the camera along its line of sight by an
// eye-space distance equivalent to a few half-sizes. This does not change where the
// point appears on screen; it only biases its depth. (Same approach as widelines.vert.)
const float DEPTH_PULL = 3.0;

vec4 pullTowardCamera(vec4 e, float pixels)
{
    vec4 c = glob.projectionMatrix * e;
    float w = max(c.w, 1e-6);

    // pixels per eye-space unit at this depth
    float pxPerUnit = abs(glob.projectionMatrix[0][0]) * 0.5 * mesh.viewport.x / w;
    float s = pixels / max(pxPerUnit, 1e-12);

    bool isOrtho = (abs(glob.projectionMatrix[3][3]) > 0.5);
    if (isOrtho)
    {
        // camera looks down the negative z-axis
        e.z += s;
    }
    else
    {
        // move along the ray through the eye point (camera is at the origin)
        float L = length(e.xyz);
        if (L > 1e-12) e.xyz *= max(1.0 - s / L, 0.5);
    }
    return e;
}

void main()
{
    vec4 e = mesh.mv * vec4(pos, 1);

    // only bias points that are larger than a pixel
    float hs = 0.5 * mesh.pointSize;
    vec4 ep = ((mesh.pointSize > 1.0) && (mesh.useDepthBias > 0) ? pullTowardCamera(e, DEPTH_PULL * hs) : e);

    vec4 r = glob.projectionMatrix * ep;
    r.z -= glob.zOffset;
    v_pos = e.xyz;
    v_col = col;
    gl_Position = r;
    gl_PointSize = mesh.pointSize;
}
