#pragma once
#include <QtCore/qmap.h>
#include "Material.h"
#include "Utility.h"
#include "FurnitureModel.h"
#include <iostream>
#include "DecorationModel.h"

class Assets
{
public:
	static Assets* GetAssetsInstance()
	{
		if (!m_assets)
		{
			m_assets = new Assets();
		}
		return m_assets;
	}
	float RoomWidth;
	float RoomHeight;
	float RoomDepth;
	QVector<FurnitureModel*> GetFurnitureModels();
	QVector<DecorationModel*> GetDecorationModels();	
	QVector<DecorationModel*> GetUpdatedDecorationModels();
	QMap<QString, QVector3D> GetColors();
	QVector3D& GetColorByName(QString &colorname);

	Material* GetMaterial(const QString materialName);	
	void AddMaterial(QString materialName, Material* material);
	QVector3D WallColor;
	QString FloorTexture;
	FurnitureModel* GetFurnitureModelByType(QString &type);
	
	QMap<FurnitureType, QVector<QString>> MaterialMap;
	void UpdateMaterialMap();

	// textures
	// textures
	QMap<FurnitureType, QMap<QString, ColorPalette*>> FurnitureTextureColors;	
	Texture* GetTexture(QString &path);

	DecorationModel* GetDecorationModel(DecorationType &decorationtype, QString cat = "");
	DecorationModel* GetDiffDecorationModel(DecorationModel* old_model);

	// decoration z orders
	QMap<DecorationType, float> DecorationZOrders;
	// decoration medial orders
	QMap<DecorationType, float> DecorationMedialOrders;
	// decoration height orders
	QMap<DecorationType, float> DecorationHOrders;
	// decoration scales
	QMap<DecorationType, float> DecorationScales;

	// update user constraints
	void UpdateHeightOrder();
	void UpdateMedialOrder();
	void UpdateZOrder();

private:
	QMap<QString, Material*> m_materials;
	QVector<FurnitureModel*> m_funitureModels;
	QVector<DecorationModel*> m_decorationModels;
	QMap<QString, QVector3D> m_colors;
	
	// decorations
	QMap<DecorationType, QList<DecorationModel*>> m_decorations;
	
	QMap<QString, Texture*> m_textures;
	Assets();	
	void init();
	void InitColorMap();
	void InitMaterialMap();
	void InitTextureColors();
	void InitDecorationZOrders();
	void InitDecorationMedialOrders();
	void InitDecorationHOrders();
	void InitDecorationScales();

	
	
	static Assets *m_assets;
};

