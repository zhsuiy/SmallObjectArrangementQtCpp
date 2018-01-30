#pragma once
#include "FurnitureModel.h"

class FloorModel:public FurnitureModel
{
public:
	FloorModel(QVector3D leftBottomBack, QVector3D rightUpFront);
	~FloorModel();
	
private:	
	QVector3D m_left_bottom_back;
	QVector3D m_right_up_front;
};
	
