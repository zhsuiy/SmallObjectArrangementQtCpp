#version 400 core

in vec2 TexCoords;

out vec4 color;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
};



uniform Material material;

void main()
{    
    color = vec4(texture(material.diffuse, TexCoords));
	//color = vec4(1.0f,0.0f,0.0f,1.0f);
}