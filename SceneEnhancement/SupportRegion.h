#pragma once
#include <QtCore/QVector>
#include <QtCore/qmap.h>
#include <QtGui/QVector3D>

class FurnitureModel;
class DecorationModel;
class QMatrix4x4;


class SupportRegion
{
public:
	SupportRegion();
	SupportRegion(float min_x, float max_x, float min_z, float max_z, float height, QMatrix4x4 modelMatrix);
	bool IsSpaceEnough() const;	
	bool TryPutDecorationModel(DecorationModel *model);
	double ArrangeDecorationModels(FurnitureModel* support, QVector<DecorationModel*> models);

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

	// collide cost
	double calculate_collide_area(QVector<DecorationModel*> models);
	double calculate_collide_area(QVector<DecorationModel*> models, QVector<FurnitureModel*> furnitureodels);
	// boundary test
	double calculate_boundary_test(QVector<DecorationModel*> models);
	// arrange decorations
	double calculate_decoration_orders(QVector<DecorationModel*> models,QMap<int, QPair<double, double>> decoration_XZ);	
	double getPairZOrderCost(QPair<double, double> back, QPair<double, double> front, bool isSame=false); // 代表这两个物体是否同一类
	double getSingleZOrderCost(QPair<double, double> xz);
	double calculate_decoration_medial_orders(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ);
	double getPairMedialOrderCost(QPair<double, double> far, QPair<double, double> near, bool isSame=false);
	double getSingleMedialOrderCost(QPair<double, double> xz);

	double calculate_same_decoration_pos(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ);
	
};
