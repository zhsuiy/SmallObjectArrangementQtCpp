#include "WallModel.h"
#include "WallFloorMesh.h"
#include "Utility.h"
#include "Global.h"
#include "Assets.h"

WallModel::WallModel( QVector3D leftBottomBack, QVector3D rightUpFront):FurnitureModel()
{	
	this->m_left_bottom_back = leftBottomBack;
	this->m_right_up_front = rightUpFront;
	
	// save materials
	Material *material1 = Utility::GetMaterialFromString(Assets::GetAssetsInstance()->MaterialMap["Wall"][0]);
	material1->Name = "Wall1";
	material_assets["Wall1"] = material1;
	Material *material2 = Utility::GetMaterialFromString(Assets::GetAssetsInstance()->MaterialMap["Wall"][1]);
	material2->Name = "Wall2";
	material_assets["Wall2"] = material2;
	Material *material3 = Utility::GetMaterialFromString(Assets::GetAssetsInstance()->MaterialMap["Wall"][2]);
	material3->Name = "Wall3";
	material_assets["Wall3"] = material3;
	
	this->meshes.push_back(new WallFloorMesh(leftBottomBack, rightUpFront, Ceiling, material1));
	this->meshes.push_back(new WallFloorMesh(leftBottomBack, rightUpFront, BackWall, material1));
	this->meshes.push_back(new WallFloorMesh(leftBottomBack, rightUpFront, LeftWall, material2));
	this->meshes.push_back(new WallFloorMesh(leftBottomBack, rightUpFront, RightWall, material3));

	this->Type = "Wall";
	init();
	
	this->LocationTypes.push_back(FurnitureLocationType::FTBack);
	this->LocationTypes.push_back(FurnitureLocationType::FTBottom);
	this->LocationTypes.push_back(FurnitureLocationType::FTLeft);
	this->updateTranslation();
	this->SetModelMatrix();
	this->UpdateBoundingBoxWorldCoordinates();

	this->OrderMaterialByMeshArea();
	this->UpdateMeshMaterials();

	

}

WallModel::~WallModel()
{
}
