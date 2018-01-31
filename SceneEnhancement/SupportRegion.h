#pragma once
#include <QtCore/QVector>
#include <QtCore/qmap.h>
#include <QtGui/QVector3D>
//#include "SmallObjectArrange.h"

//class SmallObjectArrange;
class FurnitureModel;
class DecorationModel;
class QMatrix4x4;
class SmallObjectArrange;


class SupportRegion
{
public:
	SupportRegion();
	SupportRegion(float min_x, float max_x, float min_z, float max_z, float height, QMatrix4x4 modelMatrix);
	bool IsSpaceEnough() const;	
	bool TryPutDecorationModel(DecorationModel *model);
	double ArrangeDecorationModels(FurnitureModel* support, QVector<DecorationModel*> models);
	// for active learning
	double ArrangeDecorationModels(FurnitureModel* support, QVector<DecorationModel*> models, SmallObjectArrange* arranger);
	
	void Clear();
	float MinX;
	float MaxX;
	float MinZ;
	float MaxZ;
	float Height;
	float Width;
	float Depth;	
	int ModelNum;
	QVector<DecorationModel *> m_decoration_models;
private:
	FurnitureModel *furniture;
	void updateRemainingArea();
	
	float m_area;
	float m_empty_area;

	// update decoration model coordinates
	void updateDecorationModelCoords(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ);
	void applyProposalMoves(QMap<int, QPair<double, double>> &decoration_XZ);
	
	// total cost
	double getCost(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ);
	// for active learning
	double getCost(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ, SmallObjectArrange* arranger);
	// collide cost
	double calculate_collide_area(QVector<DecorationModel*> models);
	double calculate_collide_area(QVector<DecorationModel*> models, QVector<FurnitureModel*> furnitureodels);
	// boundary test
	double calculate_boundary_test(QVector<DecorationModel*> models);
	// arrange decorations
	double calculate_decoration_depth_orders(QVector<DecorationModel*> models,QMap<int, QPair<double, double>> decoration_XZ);
	double calculate_decoration_depth_orders(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ, SmallObjectArrange* arranger);
	double getPairZOrderCost(QPair<double, double> back, QPair<double, double> front, bool isSame=false); // ���������������Ƿ�ͬһ��
	double getSingleZOrderCost(QPair<double, double> xz);
	double calculate_decoration_medial_orders(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ);
	double calculate_decoration_medial_orders(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ, SmallObjectArrange* arranger);
	double getPairMedialOrderCost(QPair<double, double> far, QPair<double, double> near, bool isSame=false);
	double getSingleMedialOrderCost(QPair<double, double> xz);

	double calculate_same_decoration_pos(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ);
	
};
