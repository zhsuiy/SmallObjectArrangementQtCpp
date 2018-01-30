#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
//layout (location = 1) in vec3 color;
//layout (location = 2) in vec2 texCoord;
//out vec3 ourColor;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projection;
void main()
{
gl_Position = projection*viewMatrix*modelMatrix*vec4(position, 1.0f);
//gl_Position = vec4(position, 1.0f);
//ourColor = color;
TexCoord = texCoord;
// if non-uniform transfrom is applied
Normal = mat3(transpose(inverse(modelMatrix))) * normal;
//Normal = normal;
FragPos = vec3(modelMatrix * vec4(position, 1.0f));


}