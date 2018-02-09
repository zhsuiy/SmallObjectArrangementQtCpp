#version 330 core

uniform int code;

out vec4 outputF;

void main()
{
	outputF = vec4(code/255.0,0.5,0.5,1);
} 
