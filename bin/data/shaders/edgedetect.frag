#version 330

uniform sampler2DRect tex0;

in vec2 texCoordVarying;

out vec4 outputColor;

const vec3 W = vec3(0.2125, 0.7154, 0.0721);

void main()
{
    //If using normalized texture coordinates, we need to divide all of the st values by these factors
    //float imageWidthFactor = textureSize(tex0, 0).x;
    //float imageHeightFactor = textureSize(tex0, 0).y;
    
    vec3 textureColor = texture(tex0, texCoordVarying).rgb;
    vec2 uv = texCoordVarying.st;
    
    vec2 stp0 =     vec2(1.0, 0.0);
    vec2 st0p =     vec2(0.0, 1.0);
    vec2 stpp =     vec2(1.0, 1.0);
    vec2 stpm =     vec2(1.0, -1.0);
    
    float i00   =   dot( textureColor, W);
    float im1m1 =   dot( texture(tex0, uv - stpp).rgb, W);
    float ip1p1 =   dot( texture(tex0, uv + stpp).rgb, W);
    float im1p1 =   dot( texture(tex0, uv - stpm).rgb, W);
    float ip1m1 =   dot( texture(tex0, uv + stpm).rgb, W);
    float im10 =    dot( texture(tex0, uv - stp0).rgb, W);
    float ip10 =    dot( texture(tex0, uv + stp0).rgb, W);
    float i0m1 =    dot( texture(tex0, uv - st0p).rgb, W);
    float i0p1 =    dot( texture(tex0, uv + st0p).rgb, W);
    
    float h = -im1p1 - 2.0 * i0p1 - ip1p1 + im1m1 + 2.0 * i0m1 + ip1m1;
    float v = -im1m1 - 2.0 * im10 - im1p1 + ip1m1 + 2.0 * ip10 + ip1p1;
    
    float mag = 1.0 - length(vec2(h, v));
    vec3 target = vec3(mag);
    
    outputColor = vec4(target, 1.0);
    //This is really cool: vec4(textureColor, 1.0);
}