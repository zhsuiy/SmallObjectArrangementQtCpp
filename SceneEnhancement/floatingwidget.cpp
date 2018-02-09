#include "floatingwidget.h"

FloatingWidget::FloatingWidget(QWidget * parent) : QWidget(parent) 
{
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowFlags(Qt::WindowStaysOnTopHint);
	setWindowFlags(Qt::Tool);
	//setAttribute(Qt::WA_TranslucentBackground, true);
	QPalette myPalette;
	QColor myColor(0, 0, 0);
	myColor.setAlphaF(0.7);
	myPalette.setBrush(backgroundRole(), myColor);
	this->setPalette(myPalette);
	this->setAutoFillBackground(true);	
}

FloatingWidget::~FloatingWidget() {
	
}
