#version 400 core
layout (location = 0) in vec3 position;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projection;
void main()
{
	gl_Position = projection*viewMatrix*modelMatrix*vec4(position, 1.0f);
}