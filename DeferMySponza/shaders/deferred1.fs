#version 330 core

layout (location = 0) out vec4 oPosition;
layout (location = 1) out vec4 oNormal;
layout (location = 2) out vec4 oColor;

uniform int useDiffTexture;
uniform sampler2D difTexture;
uniform int useNormalTexture;
uniform sampler2D normTexture;
uniform int useSpecTexture;
uniform sampler2D specTexture;
uniform vec3 albedoColor;
uniform int useMetTexture;
uniform sampler2D metTexture;
uniform float near;
uniform float far;
uniform vec3 eyePos;

in vec3 wPos;
in vec3 wNor;
in vec2 tc;

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V)
{
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye)
    vec3 map = texture(normTexture, 1.0f - tc ).xyz;
   	map = map * 255./127. - 128./127.;
    mat3 TBN = cotangent_frame(N, -V, tc);
    return normalize(TBN * map);
}

float LinearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	oPosition.xyz = wPos;
    // Store linea depth
	oPosition.w = LinearDepth(gl_FragCoord.z);
    // Normal texture
	if(useNormalTexture == 1)
	{
		oNormal.xyz = perturb_normal(wNor,wPos - eyePos);
	}
	else
	{
		oNormal.xyz = wNor;
	}
    // Diffuse/Albedo texture
	if(useDiffTexture == 1)
	{
		oColor.xyz = texture(difTexture,1.0f - tc).xyz;
	}
	else
	{
		oColor.xyz = albedoColor;	
	}
    // Spec/roughness
    if(useSpecTexture == 1)
    {
        oNormal.w = texture(specTexture,1.0f - tc).x;
    }
    else
    {
        oNormal.w = 0.0f;
    }
    // Metallic
    if(useMetTexture == 1)
    {
        oColor.w = texture(metTexture,1.0f - tc).x;
    }
    else
    {
        oColor.w = 0.0f;
    }
}