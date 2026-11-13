#version 440

// Renders wide lines as screen-aligned quads.
// Each line segment is drawn as two triangles (6 vertices). Every vertex
// carries both end points of the segment, which endpoint it belongs to,
// and on which side of the segment it lies. The vertex is then offset
// in screen space by half the line width.

// input
layout(location = 0) in vec3 posA;     // segment start point
layout(location = 1) in vec3 posB;     // segment end point
layout(location = 2) in vec3 texCoord; // texture coordinate of this vertex' endpoint
layout(location = 3) in vec4 color;    // color of this vertex' endpoint
layout(location = 4) in vec2 corner;   // x = endpoint (0 = A, 1 = B), y = side (-1 or +1)

// output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_tex;
layout(location = 2) out vec4 v_col;

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
    float lineWidth; // line width in pixels
    int useClipping;
    int useVertexColor;
    int useTexture;
    int useDepthBias;
} mesh;

// Wide lines are extruded sideways in screen space, but every pixel of the quad keeps
// the depth of the line's center. Where the surface under the line is tilted relative
// to the screen, one side of the quad then ends up behind the surface and gets hidden
// (the line looks thinner or broken). To prevent this, each end point is moved toward
// the camera along its line of sight by an eye-space distance equivalent to a few
// half-widths. This does not change where the line appears on screen; it only biases
// its depth. A factor of 3 covers surfaces tilted up to about 70 degrees.
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
    // eye coordinates
    vec4 ea = mesh.mv * vec4(posA, 1);
    vec4 eb = mesh.mv * vec4(posB, 1);

    // clip coordinates
    vec4 ca = glob.projectionMatrix * ea;
    vec4 cb = glob.projectionMatrix * eb;
    ca.z -= glob.zOffset;
    cb.z -= glob.zOffset;

    // screen coordinates (in pixels). Guard against points behind the camera.
    float wa = max(ca.w, 1e-6);
    float wb = max(cb.w, 1e-6);
    vec2 sa = (ca.xy / wa) * 0.5 * mesh.viewport;
    vec2 sb = (cb.xy / wb) * 0.5 * mesh.viewport;

    // direction and normal of the line in screen space
    vec2 d = sb - sa;
    float L = length(d);
    vec2 dir = (L > 1e-6 ? d / L : vec2(1.0, 0.0));
    vec2 nrm = vec2(-dir.y, dir.x);

    float hw = 0.5 * mesh.lineWidth;
    bool isA = (corner.x < 0.5);

    // offset in pixels: sideways by half the width, and extend the ends by
    // half the width as well (square caps) so that connected segments join without gaps.
    vec2 offset = nrm * (corner.y * hw) + dir * (isA ? -hw : hw);

    // bias the depth of this end point toward the camera (see DEPTH_PULL above)
    vec4 e = (isA ? ea : eb);
    if (mesh.useDepthBias > 0) e = pullTowardCamera(e, DEPTH_PULL * hw);
    vec4 c = glob.projectionMatrix * e;
    c.z -= glob.zOffset;

    c.xy += offset * (2.0 / mesh.viewport) * c.w;

    v_pos = (isA ? ea.xyz : eb.xyz);
    v_tex = texCoord;
    v_col = color;
    gl_Position = c;
}
