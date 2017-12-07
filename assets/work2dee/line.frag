#define PI 3.1415926

uniform float uTime;
uniform vec4 uColor;
//uniform sampler2D uTex0;

in vec2 TexCoord0;
out vec4 outColor;
        
void main( void )
{
    vec2 uv = TexCoord0;

    float th = abs(uv.y*2. - 1.);
    vec3 col = vec3(th);

    outColor = vec4(col, 1. - th);
}