#define PI 3.1415926

smooth in vec2 TexCoord0;
flat   in vec4 InstanceData;

out vec4 outColor;
        
void main( void )
{
    vec2 uv = TexCoord0;

    float th = 1 - abs(uv.y*2. - 1.);
    vec3 col = mix(InstanceData.xyz * th, vec3(1.), 1. - th);

    outColor = vec4(col, th);
}