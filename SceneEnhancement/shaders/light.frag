#version 400 core

uniform vec3 LightColor;
out vec4 color;

void main()
{
	color = vec4(LightColor,1.0f);
}