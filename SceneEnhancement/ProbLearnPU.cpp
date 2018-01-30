#include "ProbLearning.h"
#include <QtCore/qdir.h>
#include "ColorPalette.h"
#include "ClusterMethods.h"
#include <ctime>
#include <algorithm>
#include "VisualizationTool.h"

void ProbLearning::LearnPU(PUType put)
{
	m_islearned = false;
	//m_useMI = false;
	m_pu_type = put;
	//m_energy_type = F1F2;
	m_energy_type = F1F2;
	

	// 2. do statistics
	// 2.0 cluster
	if (m_cluster_type != AllSample)
	{
		// 1. process files
		ReadInfoFromLabels();
		furniture_color_clusters.clear();
		ClusterFurnitureColors(true);
	}	

	// 2.1 single furniture color	
	CalculateFurnitureColorProbPU();

	// 2.2 pairwise furniture colors
	CalculateFurniturePairwiseColorProbPU();
	//CalculateFurniturePairwiseColorProb();

	// 2.3 decoration mutual information
	// for mcmc
	//CalculateDecorationProb();
	// for submodular calculation
	CalculateDecorationProbPU();
	CalculateDecorationPairwiseProbPU();

	// 2.4 decoration and furniture color corelation
	CalculateFurnitureDecorationProbPU();

	// 3. optimization
	//MCMCSampling();
	
	//SimulatedAnnealing();

	//SimulatedAnnealingNew();
	//MCMCSamplingNew();

	MCMCMinimumCoverSelect();
	


	//ConvexMaxProduct();
	//ConvexMaxProductDecorations();
	//BruteForce();

	m_islearned = true;
	//QMap<FurnitureType,ColorPalette*> result = GetFurnitureColorPalette(1);
	//auto list = GetDecorationTypesByNumber(15);

}


void ProbLearning::CalculateFurnitureColorProbPU()
{	
	// ��ÿ��Ҿ߾��ಢͳ�Ƹ���
	for (size_t i = 0; i < m_furniture_types.size(); i++)
	{
		// ͳ��
		QMap<ClusterIndex, double> map;
		auto clustercolor = furniture_color_clusters[m_furniture_types[i]];

		QMapIterator<int, QVector<ColorPalette*>> it(clustercolor);
		double N = 0.0;
		while (it.hasNext())
		{
			it.next();
			N += it.value().size();
		}
		it.toFront();
		double max = -1, min = 1000;
		while (it.hasNext())
		{
			it.next();
			int clusterIndex = it.key();
			double P = 0.0, U = 0.0;
			double ngs = 0;
			auto colorpalettes = it.value();
			
			for (size_t j = 0; j < colorpalettes.size(); j++)
			{
				if (colorpalettes[j]->SampleType == Pos)
				{
					ngs++;
				}
			}			

			//P = ngs / (N+1);
			P = ngs > 0 ? ngs / N - 0.000001 : 0;
			//U = ngs / (colorpalettes.size()+1);
			U = ngs > 0 ? ngs / colorpalettes.size() - 0.000001 : 0;
			assert(!isnan(P));
			assert(!isnan(U));
			double score = 0.0;
			switch (m_pu_type)
			{
			case Prevalence:
				score = P;
				break;
			case Uniqueness:
				score = U;
				break;
			case PU:
				score = P*U;
				break;
			default:
				break;
			}
			map[clusterIndex] = score;
			max = score > max ? score : max;
			min = score < min ? score : min;
		}

		// normalization
		it.toFront();
		while (it.hasNext())
		{
			it.next();
			int clusterIndex = it.key();
			double score = map[clusterIndex];
			map[clusterIndex] = (score - min + 0.000001) / (max - min + 0.00001);		
		}		

		furniture_color_probs[m_furniture_types[i]] = map;
	}
}


void ProbLearning::CalculateFurniturePairwiseColorProbPU()
{
	m_furniture_types = m_para->FurnitureTypes;
	// ��¼ÿ��cluster���ж���������
	QMap<FurnitureType, QMap<ClusterIndex, int>> positive_count;
	for (size_t i = 0; i < m_furniture_types.size(); i++)
	{
		auto cluster = furniture_color_clusters[m_furniture_types[i]];
		QMap<ClusterIndex, int> number;
		QMapIterator<int, QVector<ColorPalette*>> it(cluster);
		while (it.hasNext())
		{
			it.next();
			auto cp = it.value();
			int pos_n = 0;
			for (size_t j = 0; j < cp.size(); j++)
			{
				if (cp[j]->SampleType == Pos)
				{
					pos_n++;
				}
			}
			number[it.key()] = pos_n;
		}
		positive_count[m_furniture_types[i]] = number;
	}

	QMap<QPair<FurnitureType, FurnitureType>, int> pairwise_num;
	//QMap<QPair<FurnitureType, FurnitureType>, QMap<QPair<ClusterIndex, ClusterIndex>, double>> furniture_pairwise_color_probs;
	QVector<ImageFurnitureColorType> pos_images = m_furniture_colors[1];
	QVector<ImageFurnitureColorType> neg_images = m_furniture_colors[0];
	QVector<ImageFurnitureColorType> all_images;
	for (size_t i = 0; i < pos_images.size(); i++)
	{
		all_images.push_back(pos_images[i]);
	}
	for (size_t i = 0; i < neg_images.size(); i++)
	{
		all_images.push_back(neg_images[i]);
	}

	// c(f1,f2,g1,g2)
	QMap<QPair<FurnitureType, FurnitureType>, QMap<QPair<ClusterIndex, ClusterIndex>, double>> clustersize;


	// initialize
	for (size_t i = 0; i < m_furniture_types.size(); i++)
	{
		for (size_t j = i + 1; j < m_furniture_types.size(); j++)
		{
			QMap<QPair<ClusterIndex, ClusterIndex>, double> map;
			QMap<QPair<ClusterIndex, ClusterIndex>, double> mapcluster;

			for (size_t k = 0; k < m_para->FurnitureClusterNum; k++)
			{
				for (size_t w = 0; w < m_para->FurnitureClusterNum; w++)
				{
					map[QPair<ClusterIndex, ClusterIndex>(k, w)] = 0;
					mapcluster[QPair<ClusterIndex, ClusterIndex>(k, w)] = 0;
				}
			}
			pairwise_num[QPair<FurnitureType, FurnitureType>(m_furniture_types[i], m_furniture_types[j])] = 0;
			furniture_pairwise_color_probs[QPair<FurnitureType, FurnitureType>(m_furniture_types[i], m_furniture_types[j])]
				= map;

			// initialize
			clustersize[QPair<FurnitureType, FurnitureType>(m_furniture_types[i], m_furniture_types[j])] = mapcluster;
		}
	}

	
	for (size_t i = 0; i < all_images.size(); i++)
	{
		QList<QPair<FurnitureType, FurnitureType>> keys = pairwise_num.keys();
		for (size_t j = 0; j < keys.size(); j++)
		{
			ImageFurnitureColorType colorlabels = all_images[i];
			QList<FurnitureType> furniture_types = colorlabels.keys();
			if (furniture_types.contains(keys[j].first) && furniture_types.contains(keys[j].second))
			{
				// ͬʱ�������ּҾߵ�������һ
				pairwise_num[keys[j]]++;
				int c1 = colorlabels[keys[j].first]->ClusterIndex; // cluster_index
				int c2 = colorlabels[keys[j].second]->ClusterIndex;
				if (colorlabels[keys[j].first]->SampleType == Pos &&
					colorlabels[keys[j].second]->SampleType == Pos)
				{
					furniture_pairwise_color_probs[keys[j]][QPair<ClusterIndex, ClusterIndex>(c1, c2)]++;
				}
				clustersize[keys[j]][QPair<ClusterIndex, ClusterIndex>(c1, c2)]++;				
			}
		}
	}	

	// normalization of frequency	
	QList<QPair<FurnitureType, FurnitureType>> keys = pairwise_num.keys();
	for (size_t j = 0; j < keys.size(); j++)
	{
		double max = -1, min = 1000;
		for (size_t k = 0; k < furniture_pairwise_color_probs[keys[j]].keys().size(); k++)
		{
			// cluster index pair
			auto key = furniture_pairwise_color_probs[keys[j]].keys()[k];
			double f1f2g1g2s1 = furniture_pairwise_color_probs[keys[j]][key];
			double f1f2 = pairwise_num[keys[j]] + 1;
			double f1f2g1g2 = clustersize[keys[j]][key] + 1;
			double score = 0.0;
			double P = f1f2g1g2s1 / (f1f2 + 1);
			double U = f1f2g1g2s1 / (f1f2g1g2 + 1);
			switch (m_pu_type)
			{
			case Prevalence:
				score = P;
				break;
			case Uniqueness:
				score = U;
				break;
			case PU:
				score = P*U;
				break;
			default:
				break;
			}
			max = score > max ? score : max;
			min = score < min ? score : min;
			furniture_pairwise_color_probs[keys[j]][key] = score;
		}

		// unification
		for (size_t k = 0; k < furniture_pairwise_color_probs[keys[j]].keys().size(); k++)
		{
			// cluster index pair
			auto key = furniture_pairwise_color_probs[keys[j]].keys()[k];
			double score = furniture_pairwise_color_probs[keys[j]][key];
			furniture_pairwise_color_probs[keys[j]][key] = (score - min + 0.000001) / (max - min + 0.00001);
		}
	}
}


void ProbLearning::CalculateDecorationProbPU()
{	
	m_decoration_types = m_para->DecorationTypes;
	QMap<DecorationType, int> neg_decoration_occurrence; // ��¼ÿ��С����ڸ������г��ֵĴ���
	QMap<DecorationType, int> pos_decoration_occurrence; // ��¼ÿ��С������������г��ֵĴ���
	decoration_support_probs.clear(); // С��������ڴ�Ҿߵĸ���
	for (size_t i = 0; i < m_decoration_types.size(); i++)
	{
		neg_decoration_occurrence[m_decoration_types[i]] = 0;
		pos_decoration_occurrence[m_decoration_types[i]] = 0;
		decoration_probs[m_decoration_types[i]] = 0.0;
	}
	QVector<ImageDecorationType> neg_decorations = m_decorations[0];
	QVector<ImageDecorationType> pos_decorations = m_decorations[1];

	// ��i��ͼƬ
	for (size_t i = 0; i < neg_decorations.size(); i++)
	{
		// ��i��ͼƬ�ĵ�j����ע
		for (size_t j = 0; j < neg_decorations[i].size(); j++)
		{
			if (neg_decoration_occurrence.contains(neg_decorations[i][j].first)) // DecorationType
			{
				neg_decoration_occurrence[neg_decorations[i][j].first]++;
				// ��ӵ�С����ʹ�ҾߵĹ�ϵ��
				addToDecorationSupportProb(neg_decorations[i][j]);
			}
		}
	}
	for (size_t i = 0; i < pos_decorations.size(); i++)
	{
		for (size_t j = 0; j < pos_decorations[i].size(); j++)
		{
			if (pos_decoration_occurrence.contains(pos_decorations[i][j].first))
			{
				pos_decoration_occurrence[pos_decorations[i][j].first]++;
				// ��ӵ�С����ʹ�ҾߵĹ�ϵ��
				addToDecorationSupportProb(pos_decorations[i][j]);
			}
		}
	}

	// ��С��������ڴ�Ҿ���ת���ɸ���
	QMapIterator<DecorationType, QMap<FurnitureType, double>> it(decoration_support_probs);
	while (it.hasNext())
	{
		it.next();
		auto map = it.value();
		QMapIterator<FurnitureType, double> it_inner(map);
		double totalnum = pos_decoration_occurrence[it.key()] + neg_decoration_occurrence[it.key()];
		while (it_inner.hasNext())
		{
			it_inner.next();
			map[it_inner.key()] /= totalnum;
		}
		decoration_support_probs[it.key()] = map;
	}

	// calculate prevalence and uniqueness
	decoration_probs_pu.clear();
	for (size_t i = 0; i < m_decoration_types.size(); i++)
	{
		double A = pos_decoration_occurrence[m_decoration_types[i]];
		double B = neg_decoration_occurrence[m_decoration_types[i]];
		double C = pos_decorations.size() - A;
		//double D = neg_decorations.size() - B;

		double N = pos_decorations.size() + neg_decorations.size();
		double npos = pos_decorations.size();
		double ng1s1 = A;
		double ng0s1 = C;

		double P1 = ng1s1 / (npos + 1);
		double U1 = ng1s1 / (A + B + 1);
		double P0 = ng0s1 / (npos + 1);
		double U0 = ng0s1 / (N - A - B + 1);
		double score1 = 0.0, score0 = 0.0;
		switch (m_pu_type)
		{
		case Prevalence:
			score1 = P1;
			score0 = P0;
			break;
		case Uniqueness:
			score1 = U1;
			score0 = U0;
			break;
		case PU:
			score1 = P1*U1;
			score0 = P0*U0;
			break;
		default:
			break;
		}
		QMap<int, double> map;
		map[1] = score1;
		map[0] = score0;
		decoration_probs_pu[m_decoration_types[i]] = map;
	}
}

void ProbLearning::CalculateDecorationPairwiseProbPU()
{
	QVector<ImageDecorationType> all_decorations;
	QVector<ImageDecorationType> pos_decorations = m_decorations[1];
	QVector<ImageDecorationType> neg_decorations = m_decorations[0];
	
	for (size_t i = 0; i < pos_decorations.size(); i++)
		all_decorations.push_back(pos_decorations[i]);
	for (size_t i = 0; i < neg_decorations.size(); i++)
		all_decorations.push_back(neg_decorations[i]);
	int n_pos = pos_decorations.size();
	
	// c(o1,o2)
	QMap<QPair<DecorationType, DecorationType>, int> pairwise_num; // ����һ����ֹ�����
	// c(o1,o2,s1)
	QMap<QPair<DecorationType, DecorationType>, QMap<QPair<ClusterIndex, ClusterIndex>, double>> clustersize;



	decoration_pairwise_probs_pu.clear();
	auto decorationtypes = m_para->DecorationTypes;
	for (size_t i = 0; i < decorationtypes.size(); i++)
	{
		for (size_t j = i + 1; j < decorationtypes.size(); j++)
		{
			if (!decoration_support_probs.contains(decorationtypes[i]) || 
				!decoration_support_probs.contains(decorationtypes[j]))  // ֻ���ǳ��������ݼ��е�С����
			{
				continue;
			}
			QMap<QPair<ClusterIndex, ClusterIndex>, double> map;
			QMap<QPair<ClusterIndex, ClusterIndex>, double> mapcluster;
			for (size_t k = 0; k < 2; k++)
			{
				for (size_t w = 0; w < 2; w++)
				{
					map[QPair<ClusterIndex, ClusterIndex>(k, w)] = 0;
				}
			}
			pairwise_num[QPair<DecorationType, DecorationType>(decorationtypes[i], decorationtypes[j])] = 0;
			decoration_pairwise_probs_pu[QPair<DecorationType, DecorationType>(decorationtypes[i], decorationtypes[j])]
				= map;
			clustersize[QPair<DecorationType, DecorationType>(decorationtypes[i], decorationtypes[j])] = mapcluster;
		}
	}
	for (size_t i = 0; i < all_decorations.size(); i++)
	{
		QList<QPair<DecorationType, DecorationType>> keys = decoration_pairwise_probs_pu.keys();
		for (size_t j = 0; j < keys.size(); j++)
		{
			ImageDecorationType decorationlabels = all_decorations[i];
			QList<DecorationType> decoration_types; // ��ǰͼƬ��������С�������
			for (size_t k = 0; k < decorationlabels.size(); k++)
			{
				if (!decoration_types.contains(decorationlabels[k].first))
				{
					decoration_types.push_back(decorationlabels[k].first);
				}
			}		
			int d1 = decoration_types.contains(keys[j].first) ? 1 : 0;
			int d2 = decoration_types.contains(keys[j].second) ? 1 : 0;
			if (i < n_pos) //pos example
			{
				decoration_pairwise_probs_pu[keys[j]][QPair<ClusterIndex, ClusterIndex>(d1, d2)]++;
			}
			clustersize[keys[j]][QPair<ClusterIndex, ClusterIndex>(d1, d2)]++;			
		}
	}

	// normalization of frequency

	double N = all_decorations.size();
	auto keys = decoration_pairwise_probs_pu.keys();
	for (size_t j = 0; j < keys.size(); j++)
	{
		double max = -1, min = 1000;
		for (size_t k = 0; k < decoration_pairwise_probs_pu[keys[j]].keys().size(); k++)
		{
			// cluster index pair
			auto key = decoration_pairwise_probs_pu[keys[j]].keys()[k];
			double score = 0.0;
			if (key == QPair<int, int>(0, 0)) // ��С����������ֵ����������
			{
				score = 1.0 / (N*m_para->FurnitureClusterNum);
				//furniture_decoration_probs_pu[keys[j]][key] = 1.0 / (N*m_para->FurnitureClusterNum);
			}
			else
			{
				double f1d1g1o1s1 = decoration_pairwise_probs_pu[keys[j]][key];
				double f1d1 = pairwise_num[keys[j]] + 1;
				double f1d1g1o1 = clustersize[keys[j]][key] + 1;
				double P = f1d1g1o1s1 / (f1d1 + 1);
				double U = f1d1g1o1s1 / (f1d1g1o1 + 1);
				switch (m_pu_type)
				{
				case Prevalence:
					score = P;
					break;
				case Uniqueness:
					score = U;
					break;
				case PU:
					score = P*U;
					break;
				default:
					break;
				}
			}
			max = score > max ? score : max;
			min = score < min ? score : min;
			decoration_pairwise_probs_pu[keys[j]][key] = score;
		}

		// ��һ��
		for (size_t k = 0; k < decoration_pairwise_probs_pu[keys[j]].keys().size(); k++)
		{
			auto key = decoration_pairwise_probs_pu[keys[j]].keys()[k];
			double score = decoration_pairwise_probs_pu[keys[j]][key];
			decoration_pairwise_probs_pu[keys[j]][key] = (score - min + 0.000001) / (max - min + 0.00001);
		}
	}

	//////////////////////////////////
	// old

	

	//double N = all_decorations.size();
	//auto keys = decoration_pairwise_probs_pu.keys();
	//for (size_t j = 0; j < keys.size(); j++)
	//{
	//	int n = N - decoration_pairwise_probs_pu[keys[j]][QPair<int, int>(0, 0)]; // �����㶼�����ֵ����
	//	for (size_t k = 0; k < decoration_pairwise_probs_pu[keys[j]].keys().size(); k++)
	//	{
	//		// cluster index pair
	//		auto key = decoration_pairwise_probs_pu[keys[j]].keys()[k];			
	//		if (key == QPair<int,int>(0,0))
	//		{
	//			decoration_pairwise_probs_pu[keys[j]][key] = 1; // �趼�����ֵ����Ϊ����
	//		}
	//		
	//		decoration_pairwise_probs_pu[keys[j]][key] = (decoration_pairwise_probs_pu[keys[j]][key] + 0.1) / (n + 1);
	//	}
	//}
}

void ProbLearning::CalculateFurnitureDecorationProbPU()
{
	QVector<ImageDecorationType> all_decorations;
	QVector<ImageDecorationType> pos_decorations = m_decorations[1];
	QVector<ImageDecorationType> neg_decorations = m_decorations[0];
	for (size_t i = 0; i < pos_decorations.size(); i++)
		all_decorations.push_back(pos_decorations[i]);
	for (size_t i = 0; i < neg_decorations.size(); i++)
		all_decorations.push_back(neg_decorations[i]);

	QVector<ImageFurnitureColorType> pos_images = m_furniture_colors[1];
	QVector<ImageFurnitureColorType> neg_images = m_furniture_colors[0];
	QVector<ImageFurnitureColorType> all_images;
	for (size_t i = 0; i < pos_images.size(); i++)
		all_images.push_back(pos_images[i]);
	for (size_t i = 0; i < neg_images.size(); i++)
		all_images.push_back(neg_images[i]);

	// c(fd)
	QMap<QPair<FurnitureType, DecorationType>, int> pairwise_num;
	// c(f,d,g1,o1)
	QMap<QPair<FurnitureType, DecorationType>, QMap<QPair<ClusterIndex, ClusterIndex>, double>> clustersize;

	furniture_decoration_probs_pu.clear();
	auto decorationtypes = m_para->DecorationTypes;
	m_furniture_types = m_para->FurnitureTypes;
	for (size_t i = 0; i < m_furniture_types.size(); i++)
	{		
		for (size_t j = 0; j < decorationtypes.size(); j++)
		{
			if (!decoration_support_probs.contains(decorationtypes[j]))  // ֻ���ǳ��������ݼ��е�С����
			{
				continue;
			}
			QMap<QPair<ClusterIndex, ClusterIndex>, double> map;
			QMap<QPair<ClusterIndex, ClusterIndex>, double> mapcluster;
			for (size_t k = 0; k < m_para->FurnitureClusterNum; k++)
			{
				for (size_t w = 0; w < 2; w++)
				{
					map[QPair<ClusterIndex, ClusterIndex>(k, w)] = 0;
				}
			}
			pairwise_num[QPair<FurnitureType, DecorationType>(m_furniture_types[i], decorationtypes[j])] = 0;
			furniture_decoration_probs_pu[QPair<FurnitureType, DecorationType>(m_furniture_types[i], decorationtypes[j])]
				= map;
			clustersize[QPair<FurnitureType, DecorationType>(m_furniture_types[i], decorationtypes[j])] = mapcluster;
		}
	}

	// all_decoration �� all_images�Ƿ���� ���ǣ�
	for (size_t i = 0; i < all_decorations.size(); i++)
	{
		QList<QPair<FurnitureType, DecorationType>> keys = furniture_decoration_probs_pu.keys();
		for (size_t j = 0; j < keys.size(); j++)
		{
			ImageDecorationType decorationlabels = all_decorations[i];
			ImageFurnitureColorType colorlabels = all_images[i];
			QList<FurnitureType> furniture_types = colorlabels.keys();
			QList<DecorationType> decoration_types; // ��ǰͼƬ��������С�������	
			for (size_t k = 0; k < decorationlabels.size(); k++)
			{	
				if (!decoration_types.contains(decorationlabels[k].first))
				{
					decoration_types.push_back(decorationlabels[k].first);
				}
			}

			if (furniture_types.contains(keys[j].first))
			{
				pairwise_num[keys[j]]++;
				int c = colorlabels[keys[j].first]->ClusterIndex;
				int d = decoration_types.contains(keys[j].second) ? 1 : 0;
				if (colorlabels[keys[j].first]->SampleType == Pos)
					furniture_decoration_probs_pu[keys[j]][QPair<ClusterIndex, ClusterIndex>(c, d)]++;
				clustersize[keys[j]][QPair<ClusterIndex, ClusterIndex>(c, d)]++;
			}			
		}	
	}

	// normalization of frequency

	double N = all_decorations.size();
	auto keys = furniture_decoration_probs_pu.keys();
	for (size_t j = 0; j < keys.size(); j++)
	{	
		double max = -1, min = 1000;
		for (size_t k = 0; k < furniture_decoration_probs_pu[keys[j]].keys().size(); k++)
		{
			// cluster index pair
			auto key = furniture_decoration_probs_pu[keys[j]].keys()[k];
			double score = 0.0;
			if (key.second == 0) // ��С��������ֵ����������
			{
				score = 1.0 / (N*m_para->FurnitureClusterNum);
				//furniture_decoration_probs_pu[keys[j]][key] = 1.0 / (N*m_para->FurnitureClusterNum);
			}
			else
			{
				double f1d1g1o1s1 = furniture_decoration_probs_pu[keys[j]][key];
				double f1d1 = pairwise_num[keys[j]] + 1;
				double f1d1g1o1 = clustersize[keys[j]][key] + 1;
				double P = f1d1g1o1s1 / (f1d1 + 1);
				double U = f1d1g1o1s1 / (f1d1g1o1 + 1);
				switch (m_pu_type)
				{
				case Prevalence:
					score = P;
					break;
				case Uniqueness:
					score = U;
					break;
				case PU:
					score = P*U;
					break;
				default:
					break;
				}				
			}
			max = score > max ? score : max;
			min = score < min ? score : min;
			furniture_decoration_probs_pu[keys[j]][key] = score;
		}

		// ��һ��
		for (size_t k = 0; k < furniture_decoration_probs_pu[keys[j]].keys().size(); k++)
		{
			auto key = furniture_decoration_probs_pu[keys[j]].keys()[k];
			double score = furniture_decoration_probs_pu[keys[j]][key];
			furniture_decoration_probs_pu[keys[j]][key] = (score - min + 0.000001) / (max - min + 0.00001);
		}
	}
}
