#version 330 core

uniform sampler2D colorTexture;
uniform sampler2D colorBlurTexture;
uniform sampler2D depthTexture;
uniform float near;
uniform float far;

in vec2 texCoords;

out vec4 color;

float LinearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{ 
	// Depth in 0-1 range (btw near and far planes)
	float curDepth = LinearDepth(texture(depthTexture,texCoords).x); 
	curDepth /= far;

	vec3 colBlur = texture(colorBlurTexture,texCoords).xyz;
	vec3 colSce  = texture(colorTexture,texCoords).xyz;
	//colBlur = vec3(0.0f,0.0f,0.5f);
	//colSce  = vec3(0.5f,0.0f,0.0f);

	vec3 farDof  = mix(colSce,colBlur,pow(curDepth,2.2f));
	vec3 nearDof = mix(colBlur,colSce,curDepth * 15.5f);
	
	color = vec4(farDof,1.0f);
}
