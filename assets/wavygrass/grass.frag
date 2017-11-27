#define PI 3.1415926

uniform float uTime;
uniform vec4 uColor;
//uniform sampler2D uTex0;

in vec2 TexCoord0;
out vec4 outColor;
        
void main( void )
{
    vec2 uv = TexCoord0;
    //vec4 tex = texture(uTex0, TexCoord0);
    vec3 col = uColor.xyz;
    //col *= uv.x;

    outColor = vec4(col, uColor.w);
}