#pragma once
#include <QtGui/QVector3D>
#include <QtGui/QOpenGLShader>

class Light
{
public:
	Light();
	virtual ~Light();

	Light(QVector3D ambient, QVector3D diffuse, QVector3D specular);
	virtual void SetShaderProgram(QOpenGLShaderProgram *program) = 0;

	QVector3D Ambient;
	QVector3D Diffuse;
	QVector3D Specular;	
};


class DirectionLight :public Light
{
public:
	DirectionLight(QVector3D direction, QVector3D ambient, QVector3D diffuse, QVector3D specular);
	virtual void SetShaderProgram(QOpenGLShaderProgram *program) override;
private:
	QVector3D Direction;
};


class PointLight :public Light
{
public:
	PointLight(int id, QVector3D pos, QVector3D ambient, QVector3D diffuse,
		QVector3D specular, float linear, float quadratic, float constant);
	virtual void SetShaderProgram(QOpenGLShaderProgram *program) override;
	QVector3D Position;
protected:	
	// for attenuation
	float Constant;
	float Linear;
	float Quadratic;
	int Id;
	QString getUniformName(QString attributeName) const;
};
