#pragma once

#include "Model.h"
#include "FurnitureModel.h"

#define DecorationType QString
#define DecorationName QString
#define DecorationLocType QString

enum DecorationLocationType
{
	NotSet,
	Center,
	Left,
	Right,
	Back,
	Front
};

class DecorationModel : public Model
{
public:
	DecorationModel(FurnitureType furnitureType, DecorationType decType, QVector<DecorationLocationType> locTypes,
		float scale = 1.0f, QVector3D relativeTranslate=QVector3D(0, 0, 0), QString str = "Random");
	DecorationModel(DecorationType decType, float scale, QString &path);
	virtual void Draw(QOpenGLShaderProgram *program);
	QVector<DecorationLocationType> LocationTypes;
	void SetRelativeTranslate(float tx, float ty, float tz);
	QVector3D& GetRelativeTranslate();
	DecorationType Type;
	QString Name; // 路径名称,为了换模型
	bool IsAssigned;
	FurnitureType SupportModelType;
	FurnitureModel* SupportModel;
	void SetModelMatrix();
private:	
	QVector3D m_relative_translate;
	

};


