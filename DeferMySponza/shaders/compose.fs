#version 330 core

uniform samplerCube envMap;
uniform sampler2D scene;
uniform sampler2D ambientOclu;
uniform sampler2D normalT;

in vec2 texCoords;

out vec4 color;

float Vignetting()
{
    float len = length((texCoords.xy - vec2(0.5f,0.5f))*0.9f);
    return len;
}

void main()
{ 
	vec3 normal = texture(normalT,texCoords).xyz;
	float ao = texture(ambientOclu,texCoords).x;
	vec3 sce = texture(scene,texCoords).xyz;

	vec3 ambient = pow(textureLod(envMap,normal,8).xyz,vec3(2.2f));
	vec3 a = ambient / (ambient + vec3(1.0f));
	a = pow(a,vec3(1.0f/2.2f));

    vec4 fCol = vec4((0.85f * sce) + (0.15 * ambient),1.0f) * ao;
    color = mix(fCol,vec4(0.0f,0.0f,0.0f,1.0f),Vignetting());
}
