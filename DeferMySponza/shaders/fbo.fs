#version 330 core

uniform sampler2D colorTexture;

in vec2 texCoords;

out vec4 color;

void main()
{ 
	//color = vec4(1.0,0.0,0.0,1.0);
	//color = vec4(texCoords.x,texCoords.y,0.0,1.0);
    color = texture(colorTexture, texCoords);
}
