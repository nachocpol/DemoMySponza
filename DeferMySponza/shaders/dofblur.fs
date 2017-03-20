#version 330 core

uniform sampler2D colorTexture;
uniform int blurHorizontal;

in vec2 texCoords;

out vec4 color;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{ 
    vec2 tex_offset = 1.0 / textureSize(colorTexture, 0); // gets size of single texel
    vec3 result = texture(colorTexture, texCoords).rgb * weight[0]; // current fragment's contribution
    if(blurHorizontal == 1)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(colorTexture, texCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(colorTexture, texCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(colorTexture, texCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(colorTexture, texCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    color = vec4(result, 1.0);
}
