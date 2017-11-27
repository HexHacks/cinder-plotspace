#define PI 3.1415926

uniform float uTime;
uniform mat4  ciModelViewProjection;
in vec4       ciPosition;
in vec2       ciTexCoord0;
out vec2      TexCoord0;

float offset( vec2 uv )
{
    return 0.2*cos(2.*PI*uv.x+uTime)+0.1*sin(2.*PI*uv.y-uTime);
}

void main( void )
{
    vec4 pos = ciPosition;
    //pos.y = offset(ciTexCoord0);
    gl_Position = ciModelViewProjection * pos;
    TexCoord0 = ciTexCoord0;
}