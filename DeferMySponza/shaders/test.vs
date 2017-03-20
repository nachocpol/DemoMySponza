#version 330 core

layout (location = 0) in vec3 position; 
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 wPos;
out vec3 wNor;
out vec2 tc;

void main()
{
	wPos = (model * vec4(position,1.0f)).xyz;
	wNor = normalize(mat3(transpose(inverse(model))) * normal);
	tc	 = uv;

	gl_Position = projection * view * model * vec4(position,1.0f);
}