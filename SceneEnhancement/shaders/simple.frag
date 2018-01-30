#version 400 core

struct Material {	
	sampler2D diffuse;
	sampler2D specular;	
	float shininess;
};
uniform Material material;

struct Light {
	//vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;

//in vec3 ourColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 color;
uniform sampler2D ourTexture;
uniform sampler2D ourTexture1;
uniform float lambda;



uniform vec3 objectColor;

uniform vec3 viewPos;
void main()
{
	// Ambient
	//vec3 ambient = light.ambient * material.ambient;
	
	// Diffuse
	vec3 norm = normalize(Normal);
	//vec3 lightDir = normalize(light.position - FragPos);
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm,lightDir),0.0);
	//vec3 diffuse = light.diffuse * (diff * material.diffuse);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
	
	// Specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

	
	vec3 result = ambient + diffuse + specular;
	
	color = vec4(result, 1.0f);	
}