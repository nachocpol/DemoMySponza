// bug when im inside the pont lights
// the walls on the other side of the room gets illuminated

#version 330 core

const float pi = 3.141517f;

struct Light
{
	vec3 position;
	vec3 direction;
	vec3 color;
	float angle;
	float range;
	int type;	
};

uniform Light light;
uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D albedot;
uniform vec4 wSize;
uniform vec3 camPos;

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
	
	// Tone map
	vec3 col = fCol / (fCol + vec3(1.0f));
	col = pow(col,vec3(1.0f/2.2f));
	oColor = vec4(col,1.0f);

	//oColor = vec4(vec3(roughness),1.0f);
}
