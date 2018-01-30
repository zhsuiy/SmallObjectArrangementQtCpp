#version 330 core

struct DirLight {	
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct PointLight {
	vec3 position;
	// fading effect
	float constant;
	float linear;
	float quadratic;
	// attributes
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct Material {	
	bool useAmbientMap;
	bool useDiffuseMap;
	bool useSpecularMap;
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	sampler2D ambient;
	sampler2D diffuse;
	sampler2D specular;	
	float shininess;
	float opacity;
};

#define NR_POINT_LIGHTS 2
#define NR_MATERIAL 10

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 GetAmbientColor();
vec3 GetDiffuseColor();
vec3 GetSpecularColor();

void main()
{	
	vec3 norm = abs(normalize(Normal));
	vec3 viewDir = normalize(viewPos - FragPos);
	// Phase 1: Directional lighting	
	//vec3 result = CalcDirLight(dirLight, norm, viewDir);	
	vec3 result;
	//color = vec4(result, 1.0);	
	// Phase 2: Point lights
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	// Phase 3: Spot light
	//result += CalcSpotLight(spotLight, norm, FragPos, viewDir);	
	color = vec4(result, material.opacity);		
	//average = (result.x + result.y + result.z)/3.0;
	//color = vec4(result.x,result.x,result.x,material.opacity);
	
	//color = vec4(material.opacity);	
	//color = vec4(GetDiffuseColor(),1.0);
	//float angle = abs(dot(norm,vec3(0,1,0)));
	//color = vec4(1.0-angle,0.5,angle,1);
	//color = vec4((norm),1.0);
	//color = vec4(material.useAmbientMap,material.useDiffuseMap,material.useSpecularMap, 1.0);
	//color = vec4(material.specularColor, 1.0f);
	//color = vec4(vec3(FragPos.z),1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.direction);
	// Diffuse shading
	float diff = max(abs(dot(normal, lightDir)), 0.0f);
	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(abs(dot(viewDir, reflectDir)), 0.0), material.shininess);
	// Combine results
	vec3 ambient_color = GetAmbientColor();
	vec3 diffuse_color = GetDiffuseColor();
	vec3 specular_color = GetSpecularColor();	
	
	vec3 ambient = light.ambient * ambient_color;
	vec3 diffuse = light.diffuse * diff * diffuse_color;
	vec3 specular = light.specular * spec * specular_color;
	//return (ambient + diffuse + specular);	
	//return (specular);
	return (ambient + diffuse);	
	//return normal;
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// Diffuse shading
	//float diff = max(abs(dot(normal, lightDir)), 0.65);
	float normdir = max(abs(dot(normal, lightDir)), 0.5);
	float dis = min(1 - length(light.position - fragPos)/length(light.position) + 0.5,1.0);
	float diff = min(1*normdir+0.1*dis,1);
	//float diff = min(1 - length(light.position - fragPos)/6 + 0.5,1.0);
	//min(((1 - length(light.position - fragPos)/6) + max(abs(dot(normal, lightDir))), 0.0) + 0.5,1);
	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0f;
	if (material.shininess > 0)
		spec = pow(max(abs(dot(viewDir, reflectDir)), 0.0), material.shininess);
	// Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	// Combine results
	vec3 ambient_color = GetAmbientColor();	
	vec3 diffuse_color = GetDiffuseColor();	
	vec3 specular_color = GetSpecularColor();	
	
	vec3 ambient = light.ambient * ambient_color;
	vec3 diffuse = light.diffuse * diff * diffuse_color;
	//vec3 diffuse = light.diffuse * diffuse_color;
	vec3 specular = light.specular * spec * specular_color;
	ambient *=  attenuation;
	//ambient *= 2000000;
	diffuse *= attenuation;
	specular *= attenuation;
	//return vec3(diff,diff,diff);
	//return vec3(spec,spec,spec);	
	return (ambient + diffuse);
	//return diffuse_color;
	//return diffuse;
	//return vec3(0.1,0.1,0.1);
	
}

vec3 GetAmbientColor()
{	
	vec3 result;
	if(material.useAmbientMap)
		result = vec3(texture(material.ambient, TexCoord));
	else
		result = material.ambientColor;	
	return result;
}

vec3 GetDiffuseColor()
{	
	vec3 result;
	if(material.useDiffuseMap)
		result = vec3(texture(material.diffuse, TexCoord));
	else
		result = material.diffuseColor;
	
	float grey = (result.x + result.y + result.z)/3.0;
	//return vec3(grey, grey, grey);
	return result;
}

vec3 GetSpecularColor()
{
	vec3 result;
	if(material.useSpecularMap)
		result = vec3(texture(material.specular, TexCoord));
	else
		result = material.specularColor;
	return result;
}


