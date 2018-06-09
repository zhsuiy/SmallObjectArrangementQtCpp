#include "mainwindow.h"
#include "DisplayGLWidget.h"
#include "DisplaySceneGLWidget.h"
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenuBar>
#include "ProbLearning.h"
#include "floatingwidget.h"
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	resize(1400,900);
	
	// widgets
	centralWidget = new QWidget;
	setCentralWidget(centralWidget);
	main_layout = new QHBoxLayout;

	
	ProbLearning *problearner = new ProbLearning();
	FloatingWidget *smallObjectPanel = new FloatingWidget();
	displaySceneWidget = new DisplaySceneGLWidget(problearner, smallObjectPanel);
	displaySceneWidget->setFixedWidth(1400);
	displaySceneWidget->setFixedHeight(800);
	main_layout->addWidget(displaySceneWidget);		
	setWindowTitle("Active Arrangement of Small Objects");
	centralWidget->setLayout(main_layout);	
	main_layout->addWidget(smallObjectPanel);

	// menu
	MenuFile = menuBar()->addMenu(tr("File"));
	// color
	QAction *actionSaveFurColor = MenuFile->addAction(tr("Save furniture color"));
	connect(actionSaveFurColor, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::SaveFurnitureColor);
	QAction *actionReadFurColor = MenuFile->addAction(tr("Read furniture color"));
	connect(actionReadFurColor, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::ReadFurnitureColor);
	// decoration
	QAction *actionSaveDecoration = MenuFile->addAction(tr("Save decoration models"));
	connect(actionSaveDecoration, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::SaveDecorations);
	QAction *actionReadDecoration = MenuFile->addAction(tr("Read decoration models"));
	connect(actionReadDecoration, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::ReadDecorations);
	// camera
	QAction *actionSaveCamera = MenuFile->addAction(tr("Save camera position"));
	connect(actionSaveCamera, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::SaveCamera);
	QAction *actionReadCamera = MenuFile->addAction(tr("Read camera position"));
	connect(actionReadCamera, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::ReadCamera);


	MenuLearn = menuBar()->addMenu(tr("Learning"));
	QAction *actionTrainF1 = MenuLearn->addAction(tr("TrainF1"));
	//	connect(actionTrainF1, &QAction::triggered, problearner, SLOT(&ProbLearning::Learn,F1));
	connect(actionTrainF1, &QAction::triggered, problearner, [problearner]
	{
		problearner->Learn(F1);
	});
	QAction *actionTrainF2 = MenuLearn->addAction(tr("TrainF2"));
	connect(actionTrainF2, &QAction::triggered, problearner, [problearner]
	{
		problearner->Learn(F2);
	});
	QAction *actionTrainF1F2 = MenuLearn->addAction(tr("TrainF1F2"));
	connect(actionTrainF1F2, &QAction::triggered, problearner, [problearner]
	{
		problearner->Learn(F1F2);
	});

	MenuScene = menuBar()->addMenu(tr("Scene"));
	QAction *actionDisplay= MenuScene->addAction(tr("Display"));
	connect(actionDisplay, &QAction::triggered, this, &MainWindow::OnDisplayScene);
	QAction *exportScene = MenuScene->addAction(tr("Export scene"));
	connect(exportScene, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::ExportScene);

	MenuUpdate = menuBar()->addMenu(tr("Update"));	
	QAction *actionUpdateConfig = MenuUpdate->addAction(tr("Config  (Ctrl+U)"));
	connect(actionUpdateConfig, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::UpdateConfig);
	QAction *actionUpdateMaterial = MenuUpdate->addAction(tr("Material  (Ctrl+M)"));
	connect(actionUpdateMaterial, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::UpdateMaterials);
	QAction *actionUpdateDecoration = MenuUpdate->addAction(tr("Decorations  (Ctrl+D)"));
	connect(actionUpdateDecoration, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::UpdateDecorations);
	QAction *actionUpdateMaterialByLearner = MenuUpdate->addAction(tr("Material By Learner"));
	connect(actionUpdateMaterialByLearner, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::UpdateMaterialsByLearner);
	QAction *actionUpdateParameter = MenuUpdate->addAction(tr("Update parameter"));
	connect(actionUpdateParameter, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::UpdateParameter);
	QAction *actionUpdateScales = MenuUpdate->addAction(tr("Update decoration scales"));
	connect(actionUpdateScales, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::UpdateDecorationScales);


	MenuRender = menuBar()->addMenu(tr("Render"));	
	QAction *renderObj = MenuRender->addAction(tr("Batch render objects"));
	connect(renderObj, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::RenderObjects);
	QAction *renderSelectedSmallBB = MenuRender->addAction(tr("Render selected small object bounding boxes"));
	connect(renderSelectedSmallBB, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::ToggleDrawingSelectedSmallObjectBB);
	
	MenuRender = menuBar()->addMenu(tr("Small object Arrangement"));
	QAction *initSmallObject = MenuRender->addAction(tr("Initialize small objects"));
	connect(initSmallObject, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::InitSmallObjects);
	QAction *userpref = MenuRender->addAction(tr("Only apply user preferences"));
	connect(userpref, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::OnlyApplyUserPreference);
	QAction *propagate = MenuRender->addAction(tr("Propagate user preferences"));
	connect(propagate, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::PropagateUserPreferences);
	QAction *arrange = MenuRender->addAction(tr("Arrange small objects using preference and equal matrix"));
	connect(arrange, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::ArrangeDecorationsActive);
	QAction *alighY = MenuRender->addAction(tr("Align small objects to surface"));
	connect(alighY, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::UpdateY);
	QAction *clearSmall = MenuRender->addAction(tr("Clear all states"));
	connect(clearSmall, &QAction::triggered, displaySceneWidget, &DisplaySceneGLWidget::ClearActiveSmallObjectState);

	displaySceneWidget->setFocusPolicy(Qt::StrongFocus);
	
	//

	QToolBar *toolbar = new QToolBar("toolBar", this);	
	toolbar->addAction(QIcon("./Resources/icon/save.png"), "save current scene", displaySceneWidget, &DisplaySceneGLWidget::SaveImage);
	toolbar->addAction(QIcon("./Resources/icon/texture.png"), "toggle texture (T)", displaySceneWidget, &DisplaySceneGLWidget::ToggleTexture);
	toolbar->addAction(QIcon("./Resources/icon/decoration.png"), "toggle display decorations", displaySceneWidget, &DisplaySceneGLWidget::ToggleDisplayDecorations);
	toolbar->addAction(QIcon("./Resources/icon/arrangedecoration.png"), "re-arrange decoration models", displaySceneWidget, &DisplaySceneGLWidget::RearrangeDecorations);
	toolbar->addAction(QIcon("./Resources/icon/F1.png"),"train color using unary term", problearner, [problearner]
	{
		problearner->Learn(F1);
	});
	toolbar->addAction(QIcon("./Resources/icon/F2.png"), "train color using binary term", problearner, [problearner]
	{
		problearner->Learn(F2);
	});
	toolbar->addAction(QIcon("./Resources/icon/F1F2.png"), "train color using both unary and binary terms",
		problearner, [problearner]
	{
		problearner->Learn(F1F2);
	});
	toolbar->addAction(QIcon("./Resources/icon/MIF1.png"), "train color with unary term using MI", problearner, [problearner]
	{
		problearner->LearnMI();
	});
	toolbar->addAction(QIcon("./Resources/icon/prevelance.png"),"Use prevalence",problearner,[problearner]
	{
		problearner->LearnPU(Prevalence);
	});
	toolbar->addAction(QIcon("./Resources/icon/uniqueness.png"), "Use Uniqueness", problearner, [problearner]
	{
		problearner->LearnPU(Uniqueness);
	});
	toolbar->addAction(QIcon("./Resources/icon/pu.png"), "Use both prevalence and uniqueness", problearner, [problearner]
	{
		problearner->LearnPU(PU);
	});
	toolbar->addAction(QIcon("./Resources/icon/smallobject.png"), "Only learn small objects", problearner, [problearner]
	{
		problearner->LearnSmallObjects();
	});

	toolbar->addAction(QIcon("./Resources/icon/clusterpos.png"),"Cluster only positive samples", problearner, [problearner]
	{
		problearner->ClusterFurnitureColors(false);
	});
	toolbar->addAction(QIcon("./Resources/icon/clusterall.png"), "Cluster all samples together", problearner, [problearner]
	{
		problearner->ClusterFurnitureColors(true);
	});
	toolbar->addAction(QIcon("./Resources/icon/cluster.png"),"save cluster result",problearner,[problearner]
	{
		problearner->SaveFurnitureClusterResult();
	});
	toolbar->addAction(QIcon("./Resources/icon/clusterinorder.png"), "save cluster result in order", problearner, [problearner]
	{
		problearner->SaveFurnitureClusterResultInOrder();
	});
	
	toolbar->addAction(QIcon("./Resources/icon/recolor.png"), "change furniture color (Arrow Right)", displaySceneWidget, &DisplaySceneGLWidget::UpdateMaterialsByLearner);
	toolbar->addAction(QIcon("./Resources/icon/random.png"), "randomly assign furniture colors", displaySceneWidget, &DisplaySceneGLWidget::UpdateMaterialRandom);
	toolbar->addAction(QIcon("./Resources/icon/resample.png"), "re-sample the support furniture (Arrow Up)", displaySceneWidget, &DisplaySceneGLWidget::UpdateDecorationsByLearner);
	toolbar->addAction(QIcon("./Resources/icon/random-decoration.png"), "randomly re-sample the support furniture", displaySceneWidget, &DisplaySceneGLWidget::UpdateDecorationsRandom);
	toolbar->addAction(QIcon("./Resources/icon/reinstance.png"), "re-instantiate the decoration models (Arrow Down)", displaySceneWidget, &DisplaySceneGLWidget::UpdateDecorationInstance);

	// update constraints
	toolbar->addAction(QIcon("./Resources/icon/order-medial.png"), "Update medial-order constraints", displaySceneWidget, &DisplaySceneGLWidget::UpdateMedialOrderConstraints);
	toolbar->addAction(QIcon("./Resources/icon/order-z.png"), "Update z-order constraints", displaySceneWidget, &DisplaySceneGLWidget::UpdateZOrderConstraints);
	toolbar->addAction(QIcon("./Resources/icon/order-h.png"), "Update height-order constraints", displaySceneWidget, &DisplaySceneGLWidget::UpdateHeightOrderConstraints);
	
	toolbar->addAction(QIcon("./Resources/icon/time.png"), "Average time", displaySceneWidget, &DisplaySceneGLWidget::RecordTime);
		
	addToolBar(toolbar);

	//ui.setupUi(this);
}

MainWindow::~MainWindow()
{

}

void MainWindow::OnDisplayScene()
{	
	main_layout->addWidget(displaySceneWidget); 
}

void MainWindow::OnButtonUpdateMaterialClicked()
{
}

void MainWindow::OnButtonCancelClicked()
{
}

