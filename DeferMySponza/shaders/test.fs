#version 330 core

in vec3 wPos;
in vec3 wNor;
in vec2 tc;

out vec4 fColor;

void main()
{
	vec3 lDir = vec3(0.25,0.6f,0.0f);

	// Diffuse
	float lambert = max(dot(wNor,lDir),0.0f);
	vec4 albedo = vec4(0.7f,0.7f,0.9f,1.0f);
	vec4 diffuse = albedo * lambert;

	// Ambient
	vec4 ambient = albedo * 0.1f;
	
	fColor = diffuse + ambient;
}