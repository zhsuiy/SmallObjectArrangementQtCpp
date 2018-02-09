#pragma once

#include <QGLWidget>

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "Parameter.h"
#include "WallModel.h"
#include "FloorModel.h"
#include "SmallObjectArrange.h"

class ProbLearning;

class DisplaySceneGLWidget :public QGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	DisplaySceneGLWidget(ProbLearning *learner,QWidget *parent = 0);
	~DisplaySceneGLWidget();

	void teardownGL() const;
	void keyPressEvent(QKeyEvent *event);
	public slots:
	void UpdateConfig();
	void UpdateMaterials();		
	void UpdateMaterialsByLearner();
	void UpdateMaterialRandom();
	void UpdateDecorations();
	void UpdateDecorationsByLearner();
	void UpdateDecorationsRandom();
	void UpdateDecorationInstance(); // 更换物体的实例
	void UpdateCurrentMaterials();
	void UpdateCurrentMaterialsToGray();
	void ToggleTexture();
	void ToggleDisplayDecorations();
	void RearrangeDecorations();
	void SaveImage();
	void SaveFurnitureColor();
	void SaveDecorations();
	void ReadFurnitureColor();
	void ReadDecorations();
	void SaveCamera();
	void ReadCamera();

	void UpdateParameter();

	// update constraints
	void UpdateMedialOrderConstraints();
	void UpdateZOrderConstraints();
	void UpdateHeightOrderConstraints();

	// update decoration scales
	void UpdateDecorationScales();

	// scene export
	void ExportScene();
	void RecordTime();

	// render objects
	void RenderObjects();

	// small object arrangement
	void InitSmallObjects();
	void PropagateUserPreferences();
	void ArrangeDecorationsActive();
protected:
	void initializeGL();
	void paintGL();
	void paintSelect();
	void resizeGL(int width, int height);
	

	//void keyPressEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

	// wheel event
	void wheelEvent(QWheelEvent *event);


private:
	Parameter *parameter;
	Assets* m_assets;
	ProbLearning *m_learner;

	// render
	QOpenGLBuffer m_vbo;
	QOpenGLBuffer m_ebo;
	//QOpenGLFramebufferObject *m_fbo;
	QOpenGLShaderProgram *m_program;
	QOpenGLShaderProgram *m_program_selected;
	QOpenGLVertexArrayObject light_vao;
	QOpenGLShaderProgram *light_program;

	// view matrices
	QMatrix4x4 modelMatrix;
	QMatrix4x4 viewMatrix;
	QMatrix4x4 projection;
	QMatrix4x4 model2world;

	// mouse handeling
	Qt::MouseButton mouseButton;
	QVector3D getArcBallVector(int x, int y);
	QVector2D mouseCurPos, mouseLastPos;
	QVector2D mousePressPosition;
	
	// zoom 
	Camera *camera;
	int scrollDelta = 0;

	// UI lights
	void initLights();	
	void paintLight();
	// physical lights
	QVector<Light*> Lights;

	// assets
	QVector<FurnitureModel*> furniture_models;
	QVector<DecorationModel*> decoration_models;
	QVector<Model*> models;

	// furniture color
	QMap<FurnitureType, ColorPalette*> current_colors;
	
	void printVersionInformation();

	// state info
	bool is_display_decoration;

	// determine level of mcmc
	int m_level;

	// small object arrangement
	SmallObjectArrange *small_object_arranger;

};




