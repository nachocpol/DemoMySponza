#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec2 texCoords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f); 
    texCoords = vec2(uv.x , 1.0 - uv.y);
}  