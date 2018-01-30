#pragma once

#include "FurnitureModel.h"

class WallModel:public FurnitureModel
{
public:
	WallModel(QVector3D leftBottomBack, QVector3D rightUpFront);
	~WallModel();	

private:	
	QVector3D m_left_bottom_back;
	QVector3D m_right_up_front;
};
