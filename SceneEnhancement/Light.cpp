#include "Light.h"
#include <iostream>

Light::Light()
{
}

Light::~Light()
{
}

Light::Light(QVector3D ambient, QVector3D diffuse, QVector3D specular)
{
	this->Ambient = ambient;
	this->Diffuse = diffuse;
	this->Specular = specular;
}


DirectionLight::DirectionLight(QVector3D direction, QVector3D ambient, QVector3D diffuse, QVector3D specular)
	:Light(ambient, diffuse, specular)
{
	this->Direction = direction;
}

void DirectionLight::SetShaderProgram(QOpenGLShaderProgram* program)
{	
	program->setUniformValue(program->uniformLocation("dirLight.ambient"), this->Ambient);
	program->setUniformValue(program->uniformLocation("dirLight.diffuse"), this->Diffuse);
	program->setUniformValue(program->uniformLocation("dirLight.specular"), this->Specular);
	program->setUniformValue(program->uniformLocation("dirLight.direction"), this->Direction);
}

PointLight::PointLight(int id, QVector3D pos, QVector3D ambient, QVector3D diffuse,
	QVector3D specular, float linear, float quadratic, float constant)
	:Light(ambient, diffuse, specular)
{
	this->Id = id;
	this->Position = pos;
	this->Linear = linear;
	this->Quadratic = quadratic;
	this->Constant = constant;
}

void PointLight::SetShaderProgram(QOpenGLShaderProgram* program)
{	
	program->setUniformValue(program->uniformLocation(getUniformName("ambient")), this->Ambient);
	program->setUniformValue(program->uniformLocation(getUniformName("diffuse")), this->Diffuse);
	program->setUniformValue(program->uniformLocation(getUniformName("specular")), this->Specular);
	program->setUniformValue(program->uniformLocation(getUniformName("position")), this->Position);
	program->setUniformValue(program->uniformLocation(getUniformName("constant")), this->Constant);
	program->setUniformValue(program->uniformLocation(getUniformName("linear")), this->Linear);
	program->setUniformValue(program->uniformLocation(getUniformName("quadratic")), this->Quadratic);
	
}

QString PointLight::getUniformName(QString attributeName) const
{	 	
	QString  str = QString("pointLights[%1].%2").arg(Id).arg(attributeName);
	return str;
}
