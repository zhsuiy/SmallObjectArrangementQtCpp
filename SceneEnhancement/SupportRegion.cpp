#include "SupportRegion.h"
#include <QtCore/qglobal.h>
#include <QtGui/QVector3D>
#include "DecorationModel.h"
#include "Utility.h"
#include "Assets.h"
#include <ctime>
#include "SmallObjectArrange.h"

SupportRegion::SupportRegion()
{
}

SupportRegion::SupportRegion(float min_x, float max_x, float min_z, float max_z, float height, QMatrix4x4 modelMatrix)
{
	srand(time(NULL));
	QVector3D tmplb(min_x, height, min_z);
	QVector3D tmprf(max_x, height, max_z);
	QVector3D worldLb = modelMatrix * tmplb;
	QVector3D worldRf = modelMatrix * tmprf;

	this->MinX = qMin(worldLb.x(), worldRf.x());
	this->MaxX = qMax(worldLb.x(), worldRf.x());
	this->MinZ = qMin(worldLb.z(), worldRf.z());
	this->MaxZ = qMax(worldLb.z(), worldRf.z());
	this->Height = worldLb.y();
	this->Width = MaxX - MinX;
	this->Depth = MaxZ - MinZ;
	this->ModelNum = 0;
	this->m_area = Width * Depth;
	this->m_empty_area = m_area;
}

bool SupportRegion::IsSpaceEnough() const
{
	return m_empty_area > 0.35*m_area;	
}

bool SupportRegion::TryPutDecorationModel(DecorationModel* model)
{
	float cross_area = 0;
	for (size_t i = 0; i < m_decoration_models.size(); i++)
	{
		cross_area += Utility::GetCrossArea(m_decoration_models[i]->boundingBox->WorldLeftBottomBack(), m_decoration_models[i]->boundingBox->WorldRightUpFront(),
			model->boundingBox->WorldLeftBottomBack(), model->boundingBox->WorldRightUpFront());
	}
	if (cross_area == 0)
	{
		m_decoration_models.push_back(model);
		ModelNum++;
		updateRemainingArea();
		return true;
	}
	else
	{
		return false;
	}	
}

#define XRatio double
#define ZRatio double
double SupportRegion::ArrangeDecorationModels(FurnitureModel* support, QVector<DecorationModel*> models)
{
	if (models.size() == 0)
	{
		return 0.0;
	}
	m_decoration_models = models;
	furniture = support;	
	
	srand(time(NULL));
	typedef int DecorationID;

	QList<QPair<QMap<DecorationID, QPair<XRatio, ZRatio>>, double>> all_results;
	QMap<DecorationID, QPair<XRatio, ZRatio>> decoration_pos_ratio;
	QMap<DecorationID, QPair<XRatio, ZRatio>> old_decoration_pos_ratio;
	// ��ʼ��
	for (size_t i = 0; i < models.size(); i++)
	{
		if (!decoration_pos_ratio.contains(i))
		{
			double xratio = (static_cast<double>(rand()) / (RAND_MAX));
			double zratio = (static_cast<double>(rand()) / (RAND_MAX));
			decoration_pos_ratio[i] = QPair<XRatio, ZRatio>(xratio, zratio);
		}
	}
	updateDecorationModelCoords(models, decoration_pos_ratio);
	
	if (models.size() == 1)
	{
		decoration_pos_ratio[0].first = 0.5;
		decoration_pos_ratio[0].second = 0.5;
		double F = getCost(models, decoration_pos_ratio);
		updateDecorationModelCoords(models, decoration_pos_ratio);
		return F;

	}
	double F = getCost(models, decoration_pos_ratio);
	double Fold = F;
	

	int total_iteration = 10000;
	int n = 0;
	double beta = 5.0;
	// initial
	all_results.push_back(QPair<QMap<int, QPair<XRatio, ZRatio>>, double>
		(QMap<int, QPair<XRatio, ZRatio>>(decoration_pos_ratio), F));
	while (n++ < total_iteration)
	{
		// ��¼����ǰ��״̬
		old_decoration_pos_ratio = QMap<int, QPair<XRatio, ZRatio>>(decoration_pos_ratio);
		
		// proposal
		applyProposalMoves(decoration_pos_ratio);
		// ����ratio����decoration��λ��
		updateDecorationModelCoords(models, decoration_pos_ratio);

		F = getCost(models, decoration_pos_ratio);

		double accept_rate = qMin(1.0, exp(-beta*F) / exp(-beta*Fold));
		if ((static_cast<double>(rand()) /(RAND_MAX)) < accept_rate) // ����
		{			
			Fold = F;
			// ���浱ǰ���
			all_results.push_back(QPair<QMap<int, QPair<XRatio, ZRatio>>, double>
				(QMap<int, QPair<XRatio, ZRatio>>(decoration_pos_ratio), F));
		}
		else
		{
			// ���˵�����ǰ
			decoration_pos_ratio = old_decoration_pos_ratio;
			//n--;
		}
	}
	qSort(all_results.begin(), all_results.end(), Utility::QPairSecondComparerAscending());

	// ���ݽ������decoration model��λ��
	
	decoration_pos_ratio = all_results[0].first;	
	updateDecorationModelCoords(models, decoration_pos_ratio);
	return all_results[0].second;
}

// For active learning
double SupportRegion::ArrangeDecorationModels(FurnitureModel * support, QVector<DecorationModel*> models, SmallObjectArrange * arranger)
{
	if (models.size() == 0)
	{
		return 0.0;
	}
	m_decoration_models = models;
	furniture = support;

	srand(time(NULL));
	typedef int DecorationID;

	QList<QPair<QMap<DecorationID, QPair<XRatio, ZRatio>>, double>> all_results;
	QMap<DecorationID, QPair<XRatio, ZRatio>> decoration_pos_ratio;
	QMap<DecorationID, QPair<XRatio, ZRatio>> old_decoration_pos_ratio;
	// ��ʼ��
	for (size_t i = 0; i < models.size(); i++)
	{
		if (!decoration_pos_ratio.contains(i))
		{
			double xratio = (static_cast<double>(rand()) / (RAND_MAX));
			double zratio = (static_cast<double>(rand()) / (RAND_MAX));
			decoration_pos_ratio[i] = QPair<XRatio, ZRatio>(xratio, zratio);
		}
	}
	updateDecorationModelCoords(models, decoration_pos_ratio);

	if (models.size() == 1)
	{
		decoration_pos_ratio[0].first = 0.5;
		decoration_pos_ratio[0].second = 0.5;
		double F = getCost(models, decoration_pos_ratio, arranger);
		updateDecorationModelCoords(models, decoration_pos_ratio);
		return F;

	}
	double F = getCost(models, decoration_pos_ratio, arranger);
	double Fold = F;


	int total_iteration = 10000;
	int n = 0;
	double beta = 5.0;
	// initial
	all_results.push_back(QPair<QMap<int, QPair<XRatio, ZRatio>>, double>
		(QMap<int, QPair<XRatio, ZRatio>>(decoration_pos_ratio), F));
	while (n++ < total_iteration)
	{
		// ��¼����ǰ��״̬
		old_decoration_pos_ratio = QMap<int, QPair<XRatio, ZRatio>>(decoration_pos_ratio);

		// proposal
		applyProposalMoves(decoration_pos_ratio);
		// ����ratio����decoration��λ��
		updateDecorationModelCoords(models, decoration_pos_ratio);

		F = getCost(models, decoration_pos_ratio, arranger);

		double accept_rate = qMin(1.0, exp(-beta*F) / exp(-beta*Fold));
		if ((static_cast<double>(rand()) / (RAND_MAX)) < accept_rate) // ����
		{
			Fold = F;
			// ���浱ǰ���
			all_results.push_back(QPair<QMap<int, QPair<XRatio, ZRatio>>, double>
				(QMap<int, QPair<XRatio, ZRatio>>(decoration_pos_ratio), F));
		}
		else
		{
			// ���˵�����ǰ
			decoration_pos_ratio = old_decoration_pos_ratio;
			//n--;
		}
	}
	qSort(all_results.begin(), all_results.end(), Utility::QPairSecondComparerAscending());

	// ���ݽ������decoration model��λ��

	decoration_pos_ratio = all_results[0].first;
	updateDecorationModelCoords(models, decoration_pos_ratio);
	return all_results[0].second;
}


void SupportRegion::Clear()
{
	this->m_decoration_models.clear();
	this->m_empty_area = m_area;
	ModelNum = 0;
}

void SupportRegion::updateRemainingArea()
{
	float occupied_area = 0;
	for (size_t i = 0; i < m_decoration_models.size(); i++)
	{
		occupied_area += Utility::GetCrossArea(m_decoration_models[i]->boundingBox->WorldLeftBottomBack(), m_decoration_models[i]->boundingBox->WorldRightUpFront(),
			QVector3D(MinX, Height, MinZ), QVector3D(MaxX, Height, MaxZ));
		//occupied_area += m_decoration_models[i]->boundingBox->WorldDepth() * m_decoration_models[i]->boundingBox->WorldWidth();
	}
	m_empty_area = m_area - occupied_area;
}

void SupportRegion::updateDecorationModelCoords(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ)
{
	for (size_t i = 0; i < models.size(); i++)
	{
		float centerx = furniture->boundingBox->WorldLeftBottomBack().x() + furniture->boundingBox->WorldWidth() / 2.0;
		float centerz = furniture->boundingBox->WorldLeftBottomBack().z() + furniture->boundingBox->WorldDepth() / 2.0;
		float xratio = decoration_XZ[i].first;
		float zratio = decoration_XZ[i].second;
		
		float tx = MinX + xratio*(MaxX - MinX) - centerx;
		float tz = MinZ + zratio*(MaxZ - MinZ) - centerz;

		//������ܲ���Ҫ��
		//furniture->AdaptTranslateAccord2FrontDirection(tx, tz); // ���ݼҾߵĳ������translate
		float ty = this->Height - furniture->GetTranslate().y() + abs(models[i]->boundingBox->LeftBottomBack().y())*models[i]->GetScale();
		models[i]->SetRelativeTranslate(tx, ty, tz);
	}
}

void SupportRegion::applyProposalMoves(QMap<int, QPair<double, double>> &decoration_XZ)
{
	int move = rand() % 3;
	auto keys = decoration_XZ.keys();
	double step = 0.1;
	int di = 0, dj = 0;
	double newx = 0, newz = 0;
	switch (move)
	{
	case 0: // perturb x z ratio
		di = rand() % keys.size();
		// ���Լ��ϸ�˹�ֲ�
		newx = decoration_XZ[keys[di]].first + step *(2*(static_cast<double>(rand()) / (RAND_MAX)) -1);
		newx = newx > 1 ? 1 : newx;
		newx = newx < 0 ? 0 : newx;
		decoration_XZ[keys[di]].first = newx;
		newz = decoration_XZ[keys[di]].second + step *(2 * (static_cast<double>(rand()) / (RAND_MAX)) - 1);
		newz = newz > 1 ? 1 : newz;
		newz = newz < 0 ? 0 : newz;
		decoration_XZ[keys[di]].second = newz;
		break;
	case 1: // swap x
		di = rand() % keys.size();
		dj = rand() % keys.size();
		if (di != dj)
		{
			double tmpxi = decoration_XZ[keys[di]].first;
			decoration_XZ[keys[di]].first = decoration_XZ[keys[dj]].first;
			decoration_XZ[keys[dj]].first = tmpxi;
		}
		break;
	case 2: // swap z
		di = rand() % keys.size();
		dj = rand() % keys.size();
		if (di != dj)
		{
			double tmpzi = decoration_XZ[keys[di]].second;
			decoration_XZ[keys[di]].second = decoration_XZ[keys[dj]].second;
			decoration_XZ[keys[dj]].second = tmpzi;
		}
		break;
	default:
		break;
	}

}

double SupportRegion::getCost(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ)
//double SupportRegion::getCost(QVector<DecorationModel*> models, QMap<QString, QPair<double, double>> decoration_XZ)
{
	double F = 0.0;

	// ��ײ���
	F += 10*calculate_collide_area(models);

	// ��������Ҿߵ���ײ
	F += 10 * calculate_collide_area(models, Assets::GetAssetsInstance()->GetFurnitureModels());

	// ������
	F += 10*calculate_boundary_test(models);

	// ����ǰ��order
	 F += 4 * calculate_decoration_depth_orders(models,decoration_XZ);

	// ��������order
	F += 4 * calculate_decoration_medial_orders(models, decoration_XZ);	

	// ��ͬ����λ��Ӧ���
	//F += 2 * calculate_same_decoration_pos(models, decoration_XZ);

	return F;
}

double SupportRegion::getCost(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ, SmallObjectArrange * arranger)
{
	double F = 0.0;

	// ��ײ���
	F += 10 * calculate_collide_area(models);

	// ��������Ҿߵ���ײ
	F += 10 * calculate_collide_area(models, Assets::GetAssetsInstance()->GetFurnitureModels());

	// ������
	F += 10 * calculate_boundary_test(models);

	// ����ǰ��order
	F += 4 * calculate_decoration_depth_orders(models, decoration_XZ, arranger);

	// ��������order
	F += 4 * calculate_decoration_medial_orders(models, decoration_XZ, arranger);

	// ��ͬ����λ��Ӧ���
	//F += 2 * calculate_same_decoration_pos(models, decoration_XZ);

	return F;
}

double SupportRegion::calculate_collide_area(QVector<DecorationModel*> models)
{
	double f = 0.0;
	double sum_area = 0.0;
	for (size_t i = 0; i < models.size(); i++)
	{
		DecorationModel* model1 = models[i];
		sum_area += model1->boundingBox->WorldWidth() * model1->boundingBox->WorldDepth();
		for (size_t j = i; j < models.size(); j++)
		{
			if (j == i) // ͬһ�����岻����
			{
				continue;
			}			
			DecorationModel* model2 = models[j];

			f += Utility::GetCrossArea(model1->boundingBox->WorldLeftBottomBack(), model1->boundingBox->WorldRightUpFront(),
				model2->boundingBox->WorldLeftBottomBack(), model2->boundingBox->WorldRightUpFront());
		}
	}
	f = f / sum_area; // ��һ��
	return f;
}

double SupportRegion::calculate_collide_area(QVector<DecorationModel*> models, QVector<FurnitureModel*> furnituremodels)
{
	double f = 0.0;
	double sum_area = 0.0;
	for (size_t i = 0; i < models.size(); i++)
	{
		DecorationModel* model1 = models[i];
		sum_area += model1->boundingBox->WorldWidth() * model1->boundingBox->WorldDepth();
		for (size_t j = 0; j < furnituremodels.size(); j++)
		{			
			FurnitureModel* model2 = furnituremodels[j];
			if (model2 == this->furniture || model2->Type == "Floor" 
				|| model2->Type == "Wall" || model2->Type == "Carpet"
				|| model2->Type == "FloorProxy")
			{
				continue;
			}
			if (this->furniture->Type == "BedSheet" && model2->Type == "Bed")
			{
				continue;
			}
			if (this->furniture->Type == "TvCabinet" && model2->Type == "TV")
			{
				continue;
			}
			f += Utility::GetCrossArea(model1->boundingBox->WorldLeftBottomBack(), model1->boundingBox->WorldRightUpFront(),
				model2->boundingBox->WorldLeftBottomBack(), model2->boundingBox->WorldRightUpFront());
		}
	}
	f = f / sum_area; // ��һ��
	return f;
}

double SupportRegion::calculate_boundary_test(QVector<DecorationModel*> models)
{
	double f = 0.0;
	QVector3D lb = QVector3D(MinX, Height, MinZ);
	QVector3D rf = QVector3D(MaxX, Height, MaxZ);
	double sum_area = 0.0;
	for (size_t i = 0; i < models.size(); i++)
	{
		// ��⵱ǰģ���ж��������support region���ڲ�
		double cross = Utility::GetCrossArea(models[i]->boundingBox->WorldLeftBottomBack(), models[i]->boundingBox->WorldRightUpFront(),
			lb,rf);
		// ����� - ���ڲ��ļ�Ϊ����������
		double area = models[i]->boundingBox->WorldDepth() * models[i]->boundingBox->WorldWidth();
		sum_area += area;
		double outside = area - cross;
		f = f + outside;
	}
	f = f / sum_area; // ��һ��

	return f;
}

double SupportRegion::calculate_decoration_depth_orders(QVector<DecorationModel*> models,QMap<int, QPair<double, double>> decoration_xz_ratios)
{
	double f = 0.0;
	// ���Է�������
	QList<QPair<int, double>> decoration_orders;
	QMapIterator<int, QPair<double, double>> it(decoration_xz_ratios);
	QMap<DecorationType, float> all_decoration_orders = Assets::GetAssetsInstance()->DecorationZOrders;
	while (it.hasNext())
	{
		it.next();
		if (all_decoration_orders.contains(models[it.key()]->Type))
		{
			decoration_orders.push_back(QPair<int, double>(it.key(), all_decoration_orders[models[it.key()]->Type]));
		}
	}
	// sort decoration orders;
	qSort(decoration_orders.begin(), decoration_orders.end(), Utility::QPairSecondComparerAscending());

	if (decoration_orders.size() == 0)
	{
		return f;
	}

	for (size_t i = 0; i < decoration_orders.size() - 1; i++)
	{
		double zback = decoration_orders[i].second;
		double zfront = decoration_orders[i + 1].second;
		if (zback == zfront)
		{
			double cost = getPairZOrderCost(decoration_xz_ratios[decoration_orders[i].first],
				decoration_xz_ratios[decoration_orders[i + 1].first],true);
			f += cost;
		}
		else
		{
			double cost = getPairZOrderCost(decoration_xz_ratios[decoration_orders[i].first],
				decoration_xz_ratios[decoration_orders[i + 1].first]);
			f += (zfront - zback) * cost;
		}
		
	}
	f = f / decoration_orders.last().second;


	// single
	double fs = 0.0;
	int n = decoration_orders.size();
	double mz = decoration_orders[(n - 1) / 2].second;
	for (size_t i = 0; i < decoration_orders.size(); i++)
	{
		// ���м����Ӧ�ýӽ�0.5,�ͷ��������
		fs += 1/(1+exp(abs(decoration_orders[i].second - mz)))  * getSingleZOrderCost(decoration_xz_ratios[decoration_orders[i].first]);
	}
	
	fs /= n;

	f += fs;

	return f;
}

double SupportRegion::calculate_decoration_depth_orders(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_xz_ratios, SmallObjectArrange * arranger)
{
	double f = 0.0;
	QMap<int, QPair<double, double>>::iterator it;	
	QVector<CatName> cur_cats;
	for (it = decoration_xz_ratios.begin(); it != decoration_xz_ratios.end(); ++it)
		cur_cats.push_back(models[it.key()]->Type);
	int n = cur_cats.size();

	// info
	auto all_cats_index_mapping = arranger->GetCatIndexMapping();
	auto depth_pref = arranger->GetDepthFrontProb();
	auto depth_equal = arranger->GetDepthEqualProb();
	float norm_n = 0;
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			auto cati = cur_cats[i];
			auto catj = cur_cats[j];
			if (all_cats_index_mapping.contains(cati) &&
				all_cats_index_mapping.contains(catj))
			{
				// index in active learning data
				auto index_i = all_cats_index_mapping[cati];
				auto index_j = all_cats_index_mapping[catj];
				auto equal_ij = depth_equal[index_i][index_j];
				auto depth_front_ij = depth_pref[index_i][index_j];
				// not defined
				if (equal_ij == -1 || depth_front_ij == -1)
					continue;
				
				// equal cost:
				// record how the two objects are different
				double equal_cost = getPairZOrderCost(decoration_xz_ratios[i],
					decoration_xz_ratios[j], true);
				f += equal_ij*equal_cost;
				// record how 
				// j should be back, i should be front
				// using depth_front_ij to punish how j is wrongly put in front of i
				double unequal_cost = getPairZOrderCost(decoration_xz_ratios[j],
					decoration_xz_ratios[i]);
				f += depth_front_ij* unequal_cost;
				norm_n++;
			}
		}
	}	
	f = f / norm_n;

	//// single
	//double fs = 0.0;
	//int n = decoration_orders.size();
	//double mz = decoration_orders[(n - 1) / 2].second;
	//for (size_t i = 0; i < decoration_orders.size(); i++)
	//{
	//	// ���м����Ӧ�ýӽ�0.5,�ͷ��������
	//	fs += 1 / (1 + exp(abs(decoration_orders[i].second - mz)))  * getSingleZOrderCost(decoration_xz_ratios[decoration_orders[i].first]);
	//}

	//fs /= n;

	//f += fs;

	return f;
}

double SupportRegion::getPairZOrderCost(QPair<double, double> back, QPair<double, double> front, bool isSame)
{
	FurnitureFrontDirection ffd = furniture->FurnitureFrontDirection;
	double cost = 0.0;
	double ratioback = 0;
	double ratiofront = 0;
	switch (ffd)
	{
	case Invalid:
		break;
	case XPos:	
		ratioback = back.first;
		ratiofront = front.first;
		if (!isSame && ratioback > ratiofront) // ֻ�е�����İڵ�ǰ��ȥ��ʱ�����cost
		{
			cost = 1.0/ (1.0 + exp(-(ratioback - ratiofront)));
		}		
		break;
	case XNeg:
		ratioback = back.first;
		ratiofront = front.first;
		if (!isSame && ratioback < ratiofront) // 
		{
			cost = 1.0 / (1.0 + exp(-(ratiofront - ratioback)));
		}
		break;
	case ZPos:
		ratioback = back.second;
		ratiofront = front.second;
		if (!isSame && ratioback < ratiofront)
		{
			cost = 1.0 / (1.0 + exp(-(ratiofront - ratioback)));
		}
		break;
	case ZNeg:
		ratioback = back.second;
		ratiofront = front.second;		
		if (!isSame && ratioback > ratiofront)
		{
			cost = 1.0 / (1.0 + exp(-(ratioback - ratiofront)));
		}
		break;
	default:
		break;
	}
	if (isSame && ratioback != ratiofront)
	{		
		cost = 1.0 / (1.0 + exp(-(abs(ratioback - ratiofront))));		
	}
	return cost;	
}

double SupportRegion::getSingleZOrderCost(QPair<double, double> xz)
{
	FurnitureFrontDirection ffd = furniture->FurnitureFrontDirection;
	double cost = 0.0;
	double ratio = 0;	
	switch (ffd)
	{
	case Invalid:
		break;
	case XPos:
		ratio = xz.first;
		break;
	case XNeg:
		ratio = xz.first;
		break;
	case ZPos:
		ratio = xz.second;
		break;
	case ZNeg:
		ratio = xz.second;
		break;
	default:
		break;
	}
	cost = 1.0 / (1.0 + exp(-abs(ratio - 0.5)));
	return cost;
}

double SupportRegion::calculate_decoration_medial_orders(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ)
{
	double f = 0.0;
	// ���Է�������
	QList<QPair<int, double>> decoration_orders;
	QMapIterator<int, QPair<double, double>> it(decoration_XZ);
	QMap<DecorationType, float> all_decoration_orders = Assets::GetAssetsInstance()->DecorationMedialOrders;
	while (it.hasNext())
	{
		it.next();
		if (all_decoration_orders.contains(models[it.key()]->Type))
		{
			decoration_orders.push_back(QPair<int, double>(it.key(), all_decoration_orders[models[it.key()]->Type]));
		}
	}
	// sort decoration orders;
	qSort(decoration_orders.begin(), decoration_orders.end(), Utility::QPairSecondComparerAscending());

	if (decoration_orders.size() == 0)
	{
		return f;
	}
	for (size_t i = 0; i < decoration_orders.size() - 1; i++)
	{
		double xfarmedial = decoration_orders[i].second;
		double xnearmedial = decoration_orders[i + 1].second;	
		if (xfarmedial == xnearmedial)
		{
			double cost = getPairMedialOrderCost(decoration_XZ[decoration_orders[i].first],
				decoration_XZ[decoration_orders[i + 1].first],true);
			f += cost;
		}
		else
		{
			double cost = getPairMedialOrderCost(decoration_XZ[decoration_orders[i].first],
				decoration_XZ[decoration_orders[i + 1].first]);
			f += (xnearmedial - xfarmedial) * cost;
		}
		
	}
	f = f / decoration_orders.last().second;

	// single
	double fs = 0.0;
	int n = decoration_orders.size();
	double mx = decoration_orders.last().second;
	for (size_t i = 0; i < decoration_orders.size(); i++)
	{
		// ���м����Ӧ�ýӽ�0.5,�ͷ��������
		fs += 1 / (1 + exp(abs(decoration_orders[i].second - mx)))  * getSingleMedialOrderCost(decoration_XZ[decoration_orders[i].first]);
	}

	fs = fs / n; // Ȩ��СһЩ

	f += fs;


	return f;

}

double SupportRegion::calculate_decoration_medial_orders(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_xz_ratios, SmallObjectArrange * arranger)
{
	double f = 0.0;
	QMap<int, QPair<double, double>>::iterator it;
	QVector<CatName> cur_cats;
	for (it = decoration_xz_ratios.begin(); it != decoration_xz_ratios.end(); ++it)
		cur_cats.push_back(models[it.key()]->Type);
	int n = cur_cats.size();

	// info
	auto all_cats_index_mapping = arranger->GetCatIndexMapping();
	auto medium_pref = arranger->GetMidiumMiddleProb();
	auto medium_equal = arranger->GetMediumEqualProb();
	float norm_n = 0;
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			auto cati = cur_cats[i];
			auto catj = cur_cats[j];
			if (all_cats_index_mapping.contains(cati) &&
				all_cats_index_mapping.contains(catj))
			{
				// index in active learning data
				auto index_i = all_cats_index_mapping[cati];
				auto index_j = all_cats_index_mapping[catj];
				auto equal_ij = medium_equal[index_i][index_j];
				auto medium_center_ij = medium_pref[index_i][index_j];
				// not defined
				if (equal_ij == -1 || medium_center_ij == -1)
					continue;

				// equal cost:
				// record how the two objects are different
				double equal_cost = getPairMedialOrderCost(decoration_xz_ratios[i],
					decoration_xz_ratios[j], true);
				f += equal_ij*equal_cost;
				// record how 
				// j should be far from medium, i should be near medium
				// using medium_center_ij to punish how j is wrongly put nearer than i to medium
				double unequal_cost = getPairMedialOrderCost(decoration_xz_ratios[j],
					decoration_xz_ratios[i]);
				f += medium_center_ij* unequal_cost;
				norm_n++;
			}
		}
	}
	f = f / norm_n;

	return f;
}

double SupportRegion::getPairMedialOrderCost(QPair<double, double> far_medial, QPair<double, double> near_medial, bool isSame)
{
	FurnitureFrontDirection ffd = furniture->FurnitureFrontDirection;
	double cost = 0.0;
	double ratiofar = 0;
	double rationear = 0;
	double r_far_s = 0; // ��¼ԭʼ��xλ�ã����������ͬ�����x����
	double r_near_s = 0;
	switch (ffd)
	{
	case Invalid:
		break;
	case XPos:
		ratiofar = abs(far_medial.second - 0.5);
		rationear = abs(near_medial.second - 0.5);
		r_far_s = far_medial.second;
		r_near_s = near_medial.second;
		if (!isSame && rationear > ratiofar) // ֻ�е��м�İڵ����߲���cost
		{
			cost = 1.0 / (1.0 + exp(-(rationear - ratiofar)));
		}
		break;
	case XNeg:
		ratiofar = abs(far_medial.second - 0.5);
		rationear = abs(near_medial.second - 0.5);
		r_far_s = far_medial.second;
		r_near_s = near_medial.second;
		if (!isSame && rationear > ratiofar) // 
		{
			cost = 1.0 / (1.0 + exp(-(rationear - ratiofar)));
		}
		break;
	case ZPos:
		ratiofar = abs(far_medial.first - 0.5);
		rationear = abs(near_medial.first - 0.5);
		r_far_s = far_medial.first;
		r_near_s = near_medial.first;
		if (!isSame && rationear > ratiofar)
		{
			cost = 1.0 / (1.0 + exp(-(rationear - ratiofar)));
		}
		break;
	case ZNeg:
		ratiofar = abs(far_medial.first - 0.5);
		rationear = abs(near_medial.first - 0.5);
		r_far_s = far_medial.first;
		r_near_s = near_medial.first;
		if (!isSame && rationear > ratiofar)
		{
			cost = 1.0 / (1.0 + exp(-(rationear - ratiofar)));
		}
		/*if (ratioback < ratiofront)
		{
		cost = 1.0 / (1.0 + exp(-(ratiofront - ratioback)));
		}*/
		break;
	default:
		break;
	}

	if (isSame && rationear != ratiofar)
	{
		cost = 1.0 / (1.0 + exp(-abs(rationear - ratiofar)));
	}
	return cost;
}

double SupportRegion::getSingleMedialOrderCost(QPair<double, double> xz)
{
	FurnitureFrontDirection ffd = furniture->FurnitureFrontDirection;
	double cost = 0.0;
	double ratio = 0;
	switch (ffd)
	{
	case Invalid:
		break;
	case XPos:
		ratio = abs(xz.second - 0.5);
		break;
	case XNeg:
		ratio = abs(xz.second - 0.5);
		break;
	case ZPos:
		ratio = abs(xz.first - 0.5);
		break;
	case ZNeg:
		ratio = abs(xz.first - 0.5);
		break;
	default:
		break;
	}	
	cost = 1.0 / (1.0 + exp(-abs(ratio)));	
	return cost;
}

double SupportRegion::calculate_same_decoration_pos(QVector<DecorationModel*> models, QMap<int, QPair<double, double>> decoration_XZ)
{
	double cost = 0;
	
	for (size_t i = 0; i < models.size(); i++)
	{
		for (size_t j = i; j <  models.size(); j++)
		{
			if (models[i]->Type == models[j]->Type )
			{
				auto difx = abs(decoration_XZ[i].first - decoration_XZ[j].first);
				auto difz = abs(decoration_XZ[i].second - decoration_XZ[j].second);
				cost += sqrt(difx*difx + difz*difz);
			}
		}
	}

	cost /= models.size();
	
	return cost;
	
}
