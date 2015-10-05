#version 330

in vec2 texCoordVarying;
uniform sampler2DRect tex0;

// Large values (>= 20) result in very intense quantization and LOW framerates - a value of 4 is pretty solid
int radius = 4;

out vec4 outputColor;

void main()
{
    // Get the UV coordinates of this fragment from the vertex shader - rename it uv
    vec2 uv = texCoordVarying;
    vec4 originalColor = texture(tex0, uv);
    
    // For a radius value of 8, this equals 81
    float n = float((radius + 1) * (radius + 1));
    
    // Declare two arrays, each of four vec3 variables
    vec3 m[4];
    vec3 s[4];
    
    // Zero out all of the vec3s
    // m will hold the sum of all the surrounding fragments
    // s will hold the sum of the squares of all the surrounding fragments
    for (int k = 0; k < 4; ++k)
    {
        m[k] = vec3(0.0);
        s[k] = vec3(0.0);
    }
    
    
    //--------------------------------------------------------------
    for (int j = -radius; j <= 0; ++j)
    {
        for (int i = -radius; i <= 0; ++i)
        {
            vec3 c = texture(tex0, uv + vec2(i,j)).rgb;
            m[0] += c;
            s[0] += c * c;
        }
    }
    
    //--------------------------------------------------------------
    for (int j = -radius; j <= 0; ++j)
    {
        for (int i = 0; i <= radius; ++i)
        {
            vec3 c = texture(tex0, uv + vec2(i,j)).rgb;
            m[1] += c;
            s[1] += c * c;
        }
    }
    
    //--------------------------------------------------------------
    for (int j = 0; j <= radius; ++j)
    {
        for (int i = 0; i <= radius; ++i)
        {
            vec3 c = texture(tex0, uv + vec2(i,j)).rgb;
            m[2] += c;
            s[2] += c * c;
        }
    }
    
    //--------------------------------------------------------------
    for (int j = 0; j <= radius; ++j)
    {
        for (int i = -radius; i <= 0; ++i)
        {
            vec3 c = texture(tex0, uv + vec2(i,j)).rgb;
            m[3] += c;
            s[3] += c * c;
        }
    }
    
    //--------------------------------------------------------------
    
    // 100.0f
    float min_sigma2 = 1e+2;
    
    // Iterate through all of the vec3s
    for (int k = 0; k < 4; ++k)
    {
        // Normalize m[k] by dividing it by radius * radius
        m[k] /= n;
        s[k] = abs(s[k] / n - m[k] * m[k]);
        
        float sigma2 = s[k].r + s[k].g + s[k].b;
        if (sigma2 < min_sigma2)
        {
            min_sigma2 = sigma2;
            outputColor = vec4(mix(m[k], originalColor.rgb, 0.8), 0.5);
        }
    }
}