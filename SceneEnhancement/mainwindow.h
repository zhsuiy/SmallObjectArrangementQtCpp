#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include "DisplaySceneGLWidget.h"
#include <QtWidgets/QHBoxLayout>


class MainWindow : public QMainWindow
{
	

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Q_OBJECT
	DisplaySceneGLWidget *displaySceneWidget;
	
	QHBoxLayout *main_layout;
	QWidget *centralWidget;

	QMenu *MenuUpdate;
	QMenu *MenuLearn;
	QMenu *MenuScene;
	QMenu *MenuFile;
	QMenu *MenuRender;
	

	QPushButton *ButtonUpdateMaterial;
	QPushButton *ButtonCancel;
	private slots:
	void OnDisplayScene();
	void OnButtonUpdateMaterialClicked();
	void OnButtonCancelClicked();
	//void ActionUpdateMaterialTriggered();
	//void ActionUpdateDecorationTriggered();

	
};

#endif // DISPLAYSCENE_H
