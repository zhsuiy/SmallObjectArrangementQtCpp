#pragma once
#include <QtGui/QVector3D>
#include "Texture.h"

class MaterialElement
{
public:
	MaterialElement(QVector3D color, QVector<Texture*> texes)
	{
		this->UseMap = texes.size() > 0 ? true : false;
		this->Color = color;
		this->Textures = texes;		
	}
	MaterialElement(QVector3D color)
	{
		this->UseMap = false;
		this->Color = color;
	}
	MaterialElement(QVector<Texture *> texes)
	{
		this->UseMap = true;
		this->Textures = texes;
	}
	~MaterialElement()
	{
		for (size_t i = 0; i < Textures.size(); i++)
		{
			delete Textures[i];
		}		
	}
	bool UseMap;
	QVector3D Color;
	QVector<Texture*> Textures;	
};


class Material
{
public:
	Material();
	Material(QString name, MaterialElement *ambient, MaterialElement *diffuse, MaterialElement *specular, float shininess, float opacity);
	~Material();
	QString Name;
	MaterialElement *Ambient;
	MaterialElement *Diffuse;
	MaterialElement *Specular;
	float Shininess;
	float Opacity;
	bool HasTexture;
};
