#define PI 3.1415926

uniform float uTime;
uniform mat4  ciModelViewProjection;

in vec4 vInstanceEndpoints;
in vec4 vInstanceData;

in vec4       ciPosition;
in vec2       ciTexCoord0;
smooth out vec2 TexCoord0;
flat   out vec4 InstanceData;


mat4 getRotation(in vec2 pt0, in vec2 pt1)
{
    vec2 v = normalize(pt1 - pt0);
    // er = [1, 0]
    // v . er = |v| * |er| * cos(a)
    // v . er = v.x * 1 + v.y * 0 = v.x + 0 = v.x
    // -> v.x = cos(a)

    float c = v.x; // c = cos(a);
    float a = -atan(v.y, v.x);
    float s = sin(a); // s = sin(a);

    return mat4(c,   -s,  0.0, 0.0,
                s,   c,   0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0);
}

void main( void )
{
    //vec2 hscr = vec2(1280./2., 720./2.);
    vec2 pt0 = vInstanceEndpoints.xy;
    vec2 pt1 = vInstanceEndpoints.zw;

    // Initial position is rect [0., -thick, 1., thick]
    // This means we rotate around p0
    vec4 pos = ciPosition;
    pos.x *= length(pt1 - pt0);

    pos = (getRotation(pt0, pt1) * pos) + vec4(pt0, 0., 0.);
    gl_Position = ciModelViewProjection * pos;
    
    TexCoord0 = ciTexCoord0;
    InstanceData = vInstanceData;
}