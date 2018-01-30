#pragma once
#include <QtGui/QColor>

class PercentColor:public QColor
{
public:
	PercentColor(int r, int g, int b, float percent):QColor(r,g,b)
	{
		this->Percentage = percent;
	}
	float Percentage;
};
