#pragma once
#include <QtCore/qstring.h>
#include "Light.h"
#include <QtCore/qfile.h>
#include <iostream>
#include "Global.h"
#include "Assets.h"

#define FurnitureType QString

class Parameter
{
public:
	static  Parameter* GetParameterInstance()
	{
		if (!m_parameter)
		{
			m_parameter = new Parameter();
		}
		return m_parameter;
	};
	void Update();
	QString LightDir;
	QString ColorMapPath;
	int ScreenWidth;
	int ScreenHeight;
	QString SceneTemplatePath;
	QString DecorationModelsPath;
	QString MaterialMapPath;
	QString TexturePath;
	QString DecorationScalePath;
	QVector<FurnitureType> FurnitureTypes;
	QVector<FurnitureType> FurnitureTypesUseTextures;
	QVector<FurnitureType> AllowTupFurnitures; // ָ����Щ����������Ŷ���
	QVector<FurnitureType> MultiLayerFurnitures; // ָ����Щ�����Ƕ���
	QVector<DecorationType> DecorationTypes;
	QVector<DecorationType> DecorationMultiTypes; // �����γ��ֵ�С����
	QVector<DecorationType> MultiOccurInSameFurniture; // ������ͬһ�Ҿ��϶�γ��ֵ�С����
	QString DatasetPath; // ģ��·��
	QString LabelsPath; // ��ע�����·��
	QString AdjName; // ���ݴ�
	QString DecorationZOrdersPath; // С�����ǰ��˳���ϵ
	QString DecorationMedialOrderPath; // С���忿���м��˳���ϵ
	QString DecorationHOrderPath; // С����ĸߵ�˳��
	int FurnitureClusterNum; // ÿ���Ҿߵ���ɫ���������
	int MaxSupportNumber;	// ÿ��С�������ܳ����ڼ����Ҿ���
	int DecorationNumber; // ���ֶ�����С���
	float SupportRegionPercent; // ��������ٷ�֮���پͲ��ڷ�
	int EachSupportLayerMaxModelNum; // ÿ����༸��
	bool IsDrawFurnitureBoundingBox;
	bool IsDrawDecorationBoundingBox;
	bool IsDrawFurnitureModel; // �Ƿ���ƼҾ�
	bool IsDrawDecorationModel; // �Ƿ����С����
	int SelectSampleMethodType; // ʹ�ú��ַ���ѡ��config, 0-MCMC, 1-submodular
	
private:
	Parameter();	
	~Parameter();
	void init();
	static Parameter* m_parameter;
	
};
