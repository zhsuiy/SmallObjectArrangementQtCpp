#include "Material.h"

Material::Material()
{
	this->Ambient = new MaterialElement(QVector3D(0.0, 0.0, 0.0));
	this->Diffuse = new MaterialElement(QVector3D(0.5, 0.5, 0.5));
	this->Specular = new MaterialElement(QVector3D(0.5, 0.5, 0.5));
	this->Shininess = -1.0f;	
	this->Opacity = 1.0f;
	this->HasTexture = false;
}

Material::Material(QString name, MaterialElement* ambient, MaterialElement* diffuse, 
	MaterialElement* specular, float shininess, float opacity)
{
	this->Name = name;
	this->Ambient = ambient;
	this->Diffuse = diffuse;
	this->Specular = specular;
	this->Shininess = shininess;
	this->Opacity = opacity;
	if (diffuse->Textures.size() > 0)	
		HasTexture = true;	
	else
		HasTexture = false;
}

Material::~Material()
{
	delete this->Ambient;
	delete this->Diffuse;
	delete this->Specular;
}
