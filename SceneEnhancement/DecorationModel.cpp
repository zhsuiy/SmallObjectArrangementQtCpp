#include "DecorationModel.h"
#include "Utility.h"
#include "Assets.h"
#include "Parameter.h"

DecorationModel::DecorationModel(QString furnitureType, QString decType,
	QVector<DecorationLocationType> locTypes, float scale, QVector3D relativeTranslate, QString path)
	:Model()
{
	Type = decType;
	SupportModelType = furnitureType;
	SupportModel = Assets::GetAssetsInstance()->GetFurnitureModelByType(SupportModelType);
	SupportModel->AddDecorationModel(this); // add current decoration model to furniture model

	m_translate = SupportModel->GetTranslate();
	m_rotate = SupportModel->GetRotate();
	m_scale = scale;
	//m_relative_translate = SupportModel->GetRelativePosition(this);
	m_relative_translate = relativeTranslate;

	// set locationtype
	LocationTypes =locTypes;
	QString modelPath;
	if (path.compare("Random") == 0)
		modelPath = Utility::GetDecorationModelPath(decType);
	else
		modelPath = Utility::GetDecorationModelPath(decType,path);
	this->loadModel(modelPath);
	//directory = modelPath;
	//this->updateVertexPosition();
	//m_relative_translate = SupportModel->GetRelativePosition(this);
	init(); // update normal and boundingbox
	this->SetModelMatrix(); // setup modelmatrix for rendering
	this->UpdateBoundingBoxWorldCoordinates();
	
}

DecorationModel::DecorationModel(QString decType, float scale, QString& path)
	:Model()
{
	IsAssigned = false;
	Type = decType;	
	//SupportModelType = furnitureType;
	//SupportModel = Assets::GetAssetsInstance()->GetFurnitureModelByType(SupportModelType);
	//SupportModel->AddDecorationModel(this); // add current decoration model to furniture model

	//m_translate = SupportModel->GetTranslate();
	//m_rotate = SupportModel->GetRotate();
	m_scale = scale;
	//m_relative_translate = SupportModel->GetRelativePosition(this);
	//m_relative_translate = relativeTranslate;

	// set locationtype
	//LocationTypes = locTypes;
	QString modelPath = Utility::GetDecorationModelPath(decType, path);
	Name = path;
	this->loadModel(modelPath);
	//directory = modelPath;

	init(); // update normal and boundingbox
	this->SetModelMatrix(); // setup modelmatrix for rendering
	this->UpdateBoundingBoxWorldCoordinates();
}

void DecorationModel::SetModelMatrix()
{	
	modelMatrix.setToIdentity();
	modelMatrix.translate(m_translate);
	// 旋转本来是在这里
	modelMatrix.translate(m_relative_translate);
	modelMatrix.scale(m_scale);

	// 改到了这里
	modelMatrix.rotate(m_rotate.x(), 1, 0, 0);
	modelMatrix.rotate(m_rotate.y(), 0, 1, 0);
	modelMatrix.rotate(m_rotate.z(), 0, 0, 1);
}

void DecorationModel::Draw(QOpenGLShaderProgram* program)
{
	program->setUniformValue("modelMatrix", modelMatrix);

	if (Parameter::GetParameterInstance()->IsDrawDecorationModel)
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->Draw(program);
		}
	}
	
	if (Parameter::GetParameterInstance()->IsDrawDecorationBoundingBox && boundingBox != nullptr)
	{
		boundingBox->Draw(program);
	}
}

void DecorationModel::SetRelativeTranslate(float tx, float ty, float tz)
{
	m_relative_translate.setX(tx);
	m_relative_translate.setY(ty);
	m_relative_translate.setZ(tz);
	this->SetModelMatrix();
	this->UpdateBoundingBoxWorldCoordinates();
}

QVector3D& DecorationModel::GetRelativeTranslate()
{
	return m_relative_translate;
}
