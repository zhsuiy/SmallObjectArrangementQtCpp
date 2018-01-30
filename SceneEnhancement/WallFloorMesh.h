#pragma once
#include "Model.h"

enum WallFloorType
{
	Ceiling,
	LeftWall,
	BackWall,
	RightWall,
	Floor
};

class WallFloorMesh:public Mesh
{
public:
	WallFloorMesh(QVector3D &leftBottomBack, QVector3D &rightTopFront, WallFloorType type, Material *material);
	WallFloorType wallFloorType;	
};
