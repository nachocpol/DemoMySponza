#version 330 core

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform vec3 samples[64];
uniform vec2 wSize;

in vec2 texCoords;

out vec4 FragColor;

const int kernelSize = 8;
const float radius = 10.0;
const float occlusionPower = 1.0f;


void main()
{
    const float fourinv = 1.0f / 4.0f;
    const float kernlSizeInv = 1.0f / float(kernelSize);

    vec2 noiseScale = vec2(wSize.x * fourinv, wSize.y * fourinv); 

    // Get input for SSAO algorithm
    vec3 fragPos = texture(gPositionDepth, texCoords).xyz;
    fragPos = (view * vec4(fragPos,1.0f)).xyz;

    vec3 normal = texture(gNormal, texCoords).xyz;
    normal = normalize((view * vec4(normal,0.0f)).xyz);

    vec3 randomVec = texture(texNoise, texCoords * noiseScale).xyz;

    // Create TBN 
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-space
        sample = fragPos + sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = projection * offset; 
        offset.xyz *= 1.0f/offset.w; 
        offset.xyz = offset.xyz * 0.5 + 0.5; 
        
        // get sample depth
        float sampleDepth = -texture(gPositionDepth, offset.xy).w; // Get depth value of kernel sample

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = pow(1.0 - (occlusion * kernlSizeInv),occlusionPower);
    
    FragColor = vec4(occlusion,occlusion,occlusion,1.0f);
}