#version 330 core

const float pi = 3.141517f;

const vec2 poissonValues[9] = vec2[]
(
   vec2(0.95581, -0.18159), vec2(0.50147, -0.35807), vec2(0.69607, 0.35559),
   vec2(-0.0036825, -0.59150),	vec2(0.15930, 0.089750), vec2(-0.65031, 0.058189),
   vec2(0.11915, 0.78449),	vec2(-0.34296, 0.51575), vec2(-0.60380, -0.41527)
);

struct Light
{
	vec3 position;
	vec3 direction;
	vec3 color;
	float angle;
	float range;
	int type;	
};

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 viewm;
};

uniform Light light;
uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D albedot;
uniform vec4 wSize;
uniform vec3 camPos;
uniform int castShadow;
uniform mat4 lightMatrix;
uniform sampler2D shadowT;

out vec4 oColor;

/*
	Fresnel aproximation by Schlick
*/
vec3 Fresnel(float ndv,vec3 F0,float roughness)
{
	return F0 + (max(vec3(1.0f - roughness), F0) - F0) * 
	pow(1.0f - ndv,5.0f);
}

/*
	GeoDistribution
*/
float Distribution(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;
	
    return nom / denom;
}

/*
	Geometry term
*/
float GeometryTerm(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

/*
	Geometry aproximation by Smith
*/
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometryTerm(NdotV, roughness);
    float ggx1  = GeometryTerm(NdotL, roughness);
	
    return ggx1 * ggx2;
}

/*
	Generates a pseudo random value btw 0,1
*/
float random(in vec3 seed, in float freq)
{
   float dt = dot(floor(seed * freq), vec3(53.1215, 21.1352, 9.1322));
   return fract(sin(dt) * 2105.2354);
}

/*
	Returns a random angle
*/
float randomAngle(in vec3 seed, in float freq)
{
   return random(seed, freq) * 6.283285;
}

float SampleShadow(sampler2D sMap, vec2 coords, float comp)
{
	return step(comp,texture(sMap,coords).r);
}

float SampleShadowLinear(sampler2D sMap, vec2 coords,float comp,vec2 texelSize)
{
	vec2 pixelPos = coords/texelSize + vec2(0.5);
	vec2 fracPart = fract(pixelPos);
	vec2 startTexel = (pixelPos - fracPart) * texelSize;
	
	float blTexel = SampleShadow(sMap, startTexel,comp);
	float brTexel = SampleShadow(sMap, startTexel + vec2(texelSize.x, 0.0),comp);
	float tlTexel = SampleShadow(sMap, startTexel + vec2(0.0, texelSize.y),comp);
	float trTexel = SampleShadow(sMap, startTexel + texelSize,comp);
	
	float mixA = mix(blTexel, tlTexel, fracPart.y);
	float mixB = mix(brTexel, trTexel, fracPart.y);
	
	return mix(mixA, mixB, fracPart.x);
}

float InShadow(sampler2D sMap,vec4 pLSpace,vec3 n,vec3 ld)
{
	// Projected coords
	vec3 projCoord = pLSpace.xyz / pLSpace.w;
	projCoord = projCoord * 0.5f + 0.5f;

	// Perform PCF
	float bias = max(0.00005f * (1.0f - dot(n,ld)),0.00001f);
	float curDepth = projCoord.z;
	float compValue = projCoord.z - bias;
	vec2 texelSize = vec2(1.0f,1.0f)/textureSize(sMap,0);
	float shadow = 0.0f;
	
	/*
	for(int x = -1; x <= 1; ++x)
	{
    	for(int y = -1; y <= 1; ++y)
    	{

        	//float d = texture(sMap, projCoord.xy + vec2(x, y) * texelSize).r; 
        	//d = SampleShadowLinear(sMap,projCoord.xy + vec2(x,y) * texelSize,compValue,texelSize);
        	//shadow += curDepth - bias > d ? 1.0f : 0.0f;  
    	}      
    }    
	*/

	const float k = 1.0f / 4.0f;
	for(int i=0;i<4;i++)
	{
		if(SampleShadowLinear(sMap,projCoord.xy + poissonValues[i]/700.0f,compValue,texelSize) < curDepth - bias)
		//if(texture(sMap,projCoord.xy + poissonValues[i]/1000.0f).r < curDepth - bias)
		{
			shadow += k;
		}
	}

    return shadow;
}

void main()
{ 
	// Properties
	vec2 tc = vec2(gl_FragCoord.x/wSize.z,gl_FragCoord.y/wSize.w);
	vec3 position = texture(positions,tc).xyz;
	vec3 normal = normalize(texture(normals,tc).xyz);
	vec3 albedo = pow(texture(albedot,tc).xyz,vec3(2.2f));
	vec3 view = normalize(camPos - position);
	float roughness = 1.0f - texture(normals,tc).w;
	clamp(0.01f,1.0f,roughness);
	float metallic = texture(albedot,tc).w;
	float ndv = max(dot(normal,view),0.0f);

	if(distance(light.position,position) > light.range) discard;

	// Light equation
	// Fresnel
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0,albedo,metallic);
	vec3 F = Fresnel(ndv,F0,roughness);
	// Specular&Diffuse factors
	vec3 ks = F0;
	vec3 kd = vec3(1.0f) - ks;
	kd *= 1.0f - metallic;

	vec3 ldir = normalize(light.position - position);
	vec3 h = normalize(view + ldir);
	float d = distance(position,light.position);
	float at = mix(1.0f,0.0f,d/light.range);
	vec3 radiance = (light.color * 1.0f) * at;

    // CookTorrance
    float D = Distribution(normal,h,roughness);        
    float G = GeometrySmith(normal,view,ldir,roughness);      
    vec3 nominator    = D * G * F;
    float denominator = 4.0f * max(dot(view, normal), 0.0f) * max(dot(ldir,normal), 0.0f) + 0.001f; 
    vec3 brdf = nominator / denominator;

    // Add to the total light
    float ndl = max(dot(normal, ldir), 0.0f);                
    vec3 fCol = (kd * albedo / pi + brdf) * radiance * ndl; 

	// Spot attenuation
	float theta = dot(ldir,normalize(-light.direction));
	if(theta <= 0.75f)
	{
		fCol = vec3(0.0f,0.0f,0.0f);
	}
	
	// Tone map
	vec3 col = fCol / (fCol + vec3(1.0f));
	col = pow(col,vec3(1.0f/2.2f));
	oColor = vec4(col,1.0f);
	
	// If this light casts shadow, check shadow map
	if(castShadow == 1)
	{
		//oColor = at * vec4(1.0,0.0,0.0,1.0);
		// attenuation?
		oColor *= 1.0f - InShadow(shadowT,lightMatrix * vec4(position,1.0f),normal,ldir);	
	}
	
	//oColor = vec4(vec3(roughness),1.0f);
}
