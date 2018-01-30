#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projection;

void main()
{
    gl_Position = projection * viewMatrix * modelMatrix * vec4(position, 1.0f);
    TexCoords = texCoords;
}