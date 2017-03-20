#version 330 core

uniform sampler2D colorTexture;
uniform sampler2D depthT;
uniform sampler2D normalT;

in vec2 texCoords;

out vec4 fragColor;

const int blurSize = 4; // use size of noise texture (4x4)

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(colorTexture, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(colorTexture, texCoords + offset).r;
        }
    }
    fragColor = vec4(vec3(result / (4.0 * 4.0)),1.0f);
}