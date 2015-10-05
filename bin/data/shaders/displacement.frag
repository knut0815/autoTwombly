#version 150

uniform sampler2DRect tex0; //Paint image
uniform sampler2DRect tex1; //Displacement image
uniform float time;
uniform float displacementAmountX;
uniform float displacementAmountY;

in vec2 texCoordVarying;

out vec4 outputColor;

void main()
{
    vec2 st = texCoordVarying.st;
    vec2 stOrig = st;
    
    //Scale the texture coordinates
    st *= vec2(0.3, 0.3);
    
    //First, sample the displacement map
    vec4 color = texture(tex1, st);
    vec2 st2 = stOrig + vec2( (color.r - 0.5) * displacementAmountX, (color.b - 0.5) * displacementAmountY);
    
    //Then, sample the original image, using the displaced texture coordinates
    vec4 color2 = texture(tex0, st2);
    
    outputColor = color2;
}
