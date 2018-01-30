#include "ProbLearning.h"
#include <QtCore/qdir.h>
#include "ColorPalette.h"
#include "ClusterMethods.h"
#include <ctime>
#include <algorithm>
#include "VisualizationTool.h"
#include "factor_graph.hpp"
#include "MinimizeCoverageSelection.h"

#define Epsilon 0.000000001

using namespace std;
using namespace pano;
ProbLearning::ProbLearning()
{
	m_para = Parameter::GetParameterInstance();
	m_assets = Assets::GetAssetsInstance();
	m_adj_name = m_para->AdjName;
	m_islearned = false;
	m_useMI = false;
}

void ProbLearning::Learn(EnergyType et)
{	
	m_islearned = false;
	m_useMI = false;
	m_energy_type = et;
	furniture_color_clusters.clear();

	// 1. process files
	ReadInfoFromLabels();

	// 2. do statistics
	// 2.1 single furniture color	
	CalculateFurnitureColorProb();
	
	// 2.2 pairwise furniture colors
	CalculateFurniturePairwiseColorProb();

	// 2.3 decoration mutual information
	CalculateDecorationProb();

	// 2.4 decoration pairwise information
	//CalculateDecorationPairwiseProb();

	// 2.5 decoration and furniture color corelation
	CalculateFurnitureDecorationProbPU();


	// 3. optimization
	SimulatedAnnealing();
	
	m_islearned = true;
	//QMap<FurnitureType,ColorPalette*> result = GetFurnitureColorPalette(1);
	//auto list = GetDecorationTypesByNumber(15);

}

void ProbLearning::LearnSmallObjects()
{
	m_islearned = false;
	m_useMI = false;
	//m_energy_type = et;
	furniture_color_clusters.clear();

	// 1. process files
	ReadInfoFromLabels();

	// 2.3 decoration mutual information
	CalculateDecorationProbAll();
	CalculateDecorationPairwiseProbAll();

	// 2.4 decoration pairwise information
	//CalculateDecorationPairwiseProb();

	// 2.4 decoration and furniture color corelation


	// 3. optimization
	//SimulatedAnnealing();
	//ConvexMaxProductDecorations();

	m_islearned = true;
}

bool ProbLearning::IsLearned() const
{
	return m_islearned;
}

void ProbLearning::SaveFurnitureClusterResult()
{
	if (m_islearned)
	{
		VisualizationTool::DrawAllFurnitureClusters(furniture_color_clusters);
	}
}

void ProbLearning::SaveFurnitureClusterResultInOrder()
{
	if (m_islearned)
	{
		VisualizationTool::DrawAllFurnitureClustersInOrder(furniture_color_clusters);
	}
}

void ProbLearning::ReadInfoFromLabels()
{
	// 1.1 furniture colors
	QString pathpos = m_para->LabelsPath + m_adj_name + "/pos";
	QString pathneg = m_para->LabelsPath + m_adj_name + "/neg";
	QVector<ImageFurnitureColorType> pos_colors = GetFurnitureColors(pathpos);
	QVector<ImageFurnitureColorType> neg_colors = GetFurnitureColors(pathneg);
	for (size_t i = 0; i < pos_colors.size(); i++)
	{
		auto map = pos_colors[i];
		QMapIterator<QString, ColorPalette*> it(map);
		while (it.hasNext())
		{
			it.next();
			map[it.key()]->SampleType = Pos;
		}
	}
	for (size_t i = 0; i < neg_colors.size(); i++)
	{
		auto map = neg_colors[i];
		QMapIterator<QString, ColorPalette*> it(map);
		while (it.hasNext())
		{
			it.next();
			map[it.key()]->SampleType = Neg;
		}
	}
	m_furniture_colors[0] = neg_colors;
	m_furniture_colors[1] = pos_colors;
	// 1.2 decoration info
	QVector<ImageDecorationType> pos_decorations = GetDecorations(pathpos);
	QVector<ImageDecorationType> neg_decorations = GetDecorations(pathneg);
	m_decorations[0] = neg_decorations;
	m_decorations[1] = pos_decorations;
}

QVector<ImageFurnitureColorType> ProbLearning::GetFurnitureColors(QString& path)
{
	QVector<ImageFurnitureColorType> list;
	QDir directory(path);
	if (!directory.exists())
		return list;
	QStringList names = Utility::GetFileNames(path);
	
	for (size_t i = 0; i < names.size(); i++)
	{
		QString imgpath = path + "/" + names[i];
		ImageFurnitureColorType furnitures = Utility::ReadImageFurnitureInfo(imgpath);
		list.push_back(furnitures);	
	}
	return list;
}

QVector<ImageDecorationType> ProbLearning::GetDecorations(QString& path)
{
	
	QVector<ImageDecorationType> list;
	QDir directory(path);
	if (!directory.exists())
		return list;
	QStringList names = Utility::GetFileNames(path);

	for (size_t i = 0; i < names.size(); i++)
	{
		QString imgpath = path + "/" + names[i];
		ImageDecorationType decorations = Utility::ReadImageDecorationInfo(imgpath);		
		list.push_back(decorations);
	}
	return list;
}

void ProbLearning::CalculateFurnitureColorProb()
{
	m_furniture_types = m_para->FurnitureTypes;
	QMap<FurnitureType, QVector<ColorPalette*>> furniture_color_palettes;
	// 取出所有的正样本中的颜色
	QVector<ImageFurnitureColorType> furniture_colors = m_furniture_colors[1]; // 

	for (size_t i = 0; i < furniture_colors.size(); i++)
	{
		ImageFurnitureColorType map = furniture_colors[i];
		for (size_t j = 0; j < map.keys().size();j++)
		{
			if (!furniture_color_palettes.contains(map.keys()[j]))
			{
				QVector<ColorPalette*> palettes;
				palettes.push_back(map[map.keys()[j]]);
				furniture_color_palettes[map.keys()[j]] = palettes;
			}
			else
			{
				furniture_color_palettes[map.keys()[j]].push_back(map[map.keys()[j]]);
			}
		}
	}

	// 对每类家具聚类并统计概率
	for (size_t i = 0; i < m_furniture_types.size(); i++)
	{
		// 聚类
		QVector<ColorPalette*> colors = furniture_color_palettes[m_furniture_types[i]];
		vector<vector<int>> clusters = get_furniture_clusters(m_furniture_types[i],colors);	
		// 统计
		QMap<ClusterIndex, double> map;
		for (size_t j = 0; j < clusters.size(); j++)
		{
			map[j] = static_cast<double>(clusters[j].size()) / colors.size();
		}
		furniture_color_probs[m_furniture_types[i]] = map;
	}
}

void ProbLearning::ClusterFurnitureColors(bool useall)
{
	furniture_color_clusters.clear();
	if (useall)	
		m_cluster_type = AllSample;	
	else
		m_cluster_type = PosSample;

	m_furniture_types = m_para->FurnitureTypes;

	furniture_color_palettes.clear();

	QVector<ImageFurnitureColorType> furniture_colors;
	if (useall)
	{		
		int n_neg = m_furniture_colors[0].size();
		int n_pos = m_furniture_colors[1].size();
		for (size_t i = 0; i < n_neg; i++) // neg
		{
			furniture_colors.push_back(m_furniture_colors[0][i]);
		}
		for (size_t i = 0; i < n_pos; i++) // pos
		{
			furniture_colors.push_back(m_furniture_colors[1][i]);
		}
	}
	else
		furniture_colors = m_furniture_colors[1];


	for (size_t i = 0; i < furniture_colors.size(); i++)
	{
		ImageFurnitureColorType map = furniture_colors[i];
		for (size_t j = 0; j < map.keys().size(); j++)
		{
			if (!furniture_color_palettes.contains(map.keys()[j]))
			{
				QVector<ColorPalette*> palettes;
				palettes.push_back(map[map.keys()[j]]);
				furniture_color_palettes[map.keys()[j]] = palettes;
			}
			else
			{
				furniture_color_palettes[map.keys()[j]].push_back(map[map.keys()[j]]);
			}
		}
	}

	// 对每类家具聚类并统计概率
	for (size_t i = 0; i < m_furniture_types.size(); i++)
	{
		// 聚类
		QVector<ColorPalette*> colors = furniture_color_palettes[m_furniture_types[i]];
		vector<vector<int>> clusters = get_furniture_clusters(m_furniture_types[i], colors);		
	}
	// 对每个聚类进行排序
	reorder_cluster_results();
}

vector<vector<int>> ProbLearning::get_furniture_clusters(FurnitureType furniture_type,QVector<ColorPalette*> colors)
{	
	bool is_color_order_mattered = false;
	if (furniture_type.compare("Curtain",Qt::CaseInsensitive) == 0
		|| furniture_type.compare("BedSheet", Qt::CaseInsensitive) == 0
		|| furniture_type.compare("BedPillow", Qt::CaseInsensitive) == 0)
	{
		is_color_order_mattered = true;
	}
	int color_num = colors.size();
	vector<vector<double>> distance_matrix(color_num, vector<double>(color_num, 0.0));
	for (size_t i = 0; i < color_num; i++)
	{
		for (size_t j = 0; j < color_num; j++)
		{
			double dis = ColorPalette::GetColorPaletteDistance(colors[i], colors[j], is_color_order_mattered);
			distance_matrix[i][j] = dis;
			distance_matrix[j][i] = dis;
		}
	}
	int cluster_size = m_para->FurnitureClusterNum > color_num ? color_num : m_para->FurnitureClusterNum;
	ClusterMethods cluster_methods(distance_matrix,cluster_size);
	//vector<vector<int>> cluster_results = cluster_methods.getHierarchicalClusters(HC_AVG_DISTANCE);
	//vector<vector<int>> cluster_results = cluster_methods.getHierarchicalClusters(HC_MAX_DISTANCE);
	//vector<vector<int>> cluster_results = cluster_methods.getHierarchicalClusters(HC_MIN_DISTANCE);

	//vector<vector<int>> cluster_results = cluster_methods.getKMeansClusters();
	
	vector<int> random_center;
	if (cluster_size > 0)
	{		
		int step_size = color_num / cluster_size;
		for (size_t i = 0; i < cluster_size; i++)
		{
			random_center.push_back(i*step_size);
			//random_center.push_back(i);
		}
	}

	cout << "Clustering " << furniture_type.toStdString() << endl;
	for (size_t i = 0; i < random_center.size(); i++)
	{
		auto c = colors[random_center[i]]->Colors[0];
		cout << i << ":" << c.red() << " " << c.green() << " " << c.blue() << endl;
	}
	
	vector<vector<int>> cluster_results = cluster_methods.getKMedoidsClusters(1000,random_center);
	
	//vector<vector<int>> cluster_results = cluster_methods.getSpectralClusters(1000,color_num/4,1);
	
	// 记录cluster	
	if (!furniture_color_clusters.contains(furniture_type))
	{
		QMap<ClusterIndex, QVector<ColorPalette*>> map;
		for (size_t i = 0; i < cluster_results.size(); i++)
		{
			QVector<ColorPalette*> colorpalettes;
			for (size_t j = 0; j < cluster_results[i].size(); j++)
			{
				// 记录每个颜色所属的cluster
				colors[cluster_results[i][j]]->ClusterIndex = i;
				// 记录每个cluster对应的颜色
				colorpalettes.push_back(colors[cluster_results[i][j]]);
			}
			map[i] = colorpalettes;			
		}
		furniture_color_clusters[furniture_type] = map;	
	}	
	return cluster_results;
}

void ProbLearning::reorder_cluster_results()
{
	furniture_color_clusters_ordered.clear();
	auto keys = furniture_color_clusters.keys();
	for (size_t k = 0; k < keys.size(); k++)
	{
		auto map = furniture_color_clusters[keys[k]];
		QMap<int, QVector<ColorPalette*>> ordered_map;
		QMapIterator<int, QVector<ColorPalette*>> it(map);
		while (it.hasNext())
		{
			it.next();
			QList<QPair<int, double>> distances;
			QVector<ColorPalette*> ordered_cps;
			auto cps = it.value();
			for (size_t i = 0; i < cps.size(); i++) // 当前 cluster
			{
				double d = 0;
				for (size_t j = 0; j < cps.size(); j++)
				{
					d += ColorPalette::GetColorPaletteDistance(cps[i], cps[j]);
				}
				distances.push_back(QPair<int, double>(i, d));
			}
			qSort(distances.begin(), distances.end(), Utility::QPairSecondComparerAscending());
			for (size_t i = 0; i < distances.size(); i++)
			{
				ordered_cps.push_back(cps[distances[i].first]);
			}
			ordered_map[it.key()] = ordered_cps;
		}
		furniture_color_clusters_ordered[keys[k]] = ordered_map;
	}
}

void ProbLearning::CalculateFurniturePairwiseColorProb()
{
	QMap<QPair<FurnitureType, FurnitureType>, int> pairwise_num;
	//QMap<QPair<FurnitureType, FurnitureType>, QMap<QPair<ClusterIndex, ClusterIndex>, double>> furniture_pairwise_color_probs;
	//QVector<ImageFurnitureColorType> pos_images = m_furniture_colors[1];

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

	m_furniture_types = m_para->FurnitureTypes;
	for (size_t i = 0; i < m_furniture_types.size(); i++)
	{
		for (size_t j = i + 1; j < m_furniture_types.size(); j++)
		{
			QMap<QPair<ClusterIndex, ClusterIndex>, double> map;
			for (size_t k = 0; k < m_para->FurnitureClusterNum; k++)
			{
				for (size_t w = 0; w < m_para->FurnitureClusterNum; w++)
				{
					map[QPair<ClusterIndex, ClusterIndex>(k, w)] = 0;
				}
			}
			pairwise_num[QPair<FurnitureType, FurnitureType>(m_furniture_types[i], m_furniture_types[j])] = 0;
			furniture_pairwise_color_probs[QPair<FurnitureType, FurnitureType>(m_furniture_types[i], m_furniture_types[j])]
				= map;
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
				// 同时有这两种家具的总数加一
				pairwise_num[keys[j]]++;
				int c1 = colorlabels[keys[j].first]->ClusterIndex; // cluster_index
				int c2 = colorlabels[keys[j].second]->ClusterIndex;
				furniture_pairwise_color_probs[keys[j]][QPair<ClusterIndex, ClusterIndex>(c1, c2)]++;
			}
		}
	}

	// normalization of frequency	
	QList<QPair<FurnitureType, FurnitureType>> keys = pairwise_num.keys();
	for (size_t j = 0; j < keys.size(); j++)
	{
		for (size_t k = 0; k < furniture_pairwise_color_probs[keys[j]].keys().size(); k++)
		{
			// cluster index pair
			auto key = furniture_pairwise_color_probs[keys[j]].keys()[k]; 
			//furniture_pairwise_color_probs[keys[j]][key] = (furniture_pairwise_color_probs[keys[j]][key] + 0.000001)/ (pairwise_num[keys[j]] + 1);
			furniture_pairwise_color_probs[keys[j]][key] = (furniture_pairwise_color_probs[keys[j]][key] + 0.000001) / all_images.size();
		}			
	}	
}

void ProbLearning::CalculateDecorationProbAll()
{
	m_decoration_types = m_para->DecorationTypes;
	QMap<DecorationType, int> decoration_occurrence; // 记录每个小物件在样本中出现的次数
	decoration_support_probs.clear(); // 小物体出现在大家具的概率	
	for (size_t i = 0; i < m_decoration_types.size(); i++)
	{
		decoration_occurrence[m_decoration_types[i]] = 0;
		decoration_probs[m_decoration_types[i]] = 0.0;
	}
	QVector<ImageDecorationType> all_decorations;
	for (size_t j = 0; j < 2; j++)
	{
		for (size_t i = 0; i < m_decorations[j].size(); i++)
		{
			all_decorations.push_back(m_decorations[j][i]);
		}
	}
	double N = all_decorations.size();
	int maxocc = 0;
	// 第i张图片
	for (size_t i = 0; i < all_decorations.size(); i++)
	{
		// 第i张图片的第j个标注
		for (size_t j = 0; j < all_decorations[i].size(); j++)
		{
			if (decoration_occurrence.contains(all_decorations[i][j].first)) // DecorationType
			{
				decoration_occurrence[all_decorations[i][j].first]++;
				if (maxocc < decoration_occurrence[all_decorations[i][j].first])
				{
					maxocc = decoration_occurrence[all_decorations[i][j].first];
				}
				// 添加到小物件和大家具的关系中
				addToDecorationSupportProb(all_decorations[i][j]);
			}			
		}
	}

	// 把小物件出现在大家具上转换成概率
	QMapIterator<DecorationType, QMap<FurnitureType, double>> it(decoration_support_probs);
	while (it.hasNext())
	{
		it.next();
		auto map = it.value();
		QMapIterator<FurnitureType, double> it_inner(map);
		double totalnum = decoration_occurrence[it.key()];
		while (it_inner.hasNext())
		{
			it_inner.next();
			map[it_inner.key()] /= totalnum;
		}
		decoration_support_probs[it.key()] = map;
	}

	decoration_probs_pu.clear();
	for (size_t i = 0; i < m_decoration_types.size(); i++)
	{
		double score1 = ((double)decoration_occurrence[m_decoration_types[i]] + 0.1)/ (maxocc+1);
		double score0 = 1.0 - score1;
		//double score0 = 1 / (1 + exp(-(N - decoration_occurrence[m_decoration_types[i]])));
		QMap<int, double> map;
		map[1] = score1;
		map[0] = score0;
		decoration_probs_pu[m_decoration_types[i]] = map;
		decoration_probs[m_decoration_types[i]] = score1;
	}

	sorted_decoration_types.clear();
	auto keys = decoration_probs.keys();
	for (size_t i = 0; i < keys.size(); i++)
	{
		sorted_decoration_types.push_back(QPair<DecorationType, double>(keys[i], decoration_probs[keys[i]]));
	}
	qSort(sorted_decoration_types.begin(), sorted_decoration_types.end(), Utility::QPairSecondComparer());

}

void ProbLearning::CalculateDecorationPairwiseProbAll()
{
	QVector<ImageDecorationType> all_decorations;
	QVector<ImageDecorationType> pos_decorations = m_decorations[1];
	QVector<ImageDecorationType> neg_decorations = m_decorations[0];

	for (size_t i = 0; i < pos_decorations.size(); i++)
		all_decorations.push_back(pos_decorations[i]);
	for (size_t i = 0; i < neg_decorations.size(); i++)
		all_decorations.push_back(neg_decorations[i]);

	decoration_pairwise_probs_pu.clear();
	auto decorationtypes = m_para->DecorationTypes;
	for (size_t i = 0; i < decorationtypes.size(); i++)
	{
		for (size_t j = i + 1; j < decorationtypes.size(); j++)
		{
			if (!decoration_support_probs.contains(decorationtypes[i]) ||
				!decoration_support_probs.contains(decorationtypes[j]))  // 只考虑出现在数据集中的小物体
			{
				continue;
			}
			QMap<QPair<ClusterIndex, ClusterIndex>, double> map;
			for (size_t k = 0; k < 2; k++)
			{
				for (size_t w = 0; w < 2; w++)
				{
					map[QPair<ClusterIndex, ClusterIndex>(k, w)] = 0;
				}
			}
			decoration_pairwise_probs_pu[QPair<FurnitureType, FurnitureType>(decorationtypes[i], decorationtypes[j])]
				= map;
		}
	}
	for (size_t i = 0; i < all_decorations.size(); i++)
	{
		QList<QPair<DecorationType, DecorationType>> keys = decoration_pairwise_probs_pu.keys();
		for (size_t j = 0; j < keys.size(); j++)
		{

			ImageDecorationType decorationlabels = all_decorations[i];
			QList<DecorationType> decoration_types; // 当前图片所包含的小物体类别
			for (size_t k = 0; k < decorationlabels.size(); k++)
			{
				if (!decoration_types.contains(decorationlabels[k].first))
				{
					decoration_types.push_back(decorationlabels[k].first);
				}
			}
			int d1 = decoration_types.contains(keys[j].first) ? 1 : 0;
			int d2 = decoration_types.contains(keys[j].second) ? 1 : 0;
			decoration_pairwise_probs_pu[keys[j]][QPair<ClusterIndex, ClusterIndex>(d1, d2)]++;
		}
	}

	// normalization of frequency

	double N = all_decorations.size();
	auto keys = decoration_pairwise_probs_pu.keys();
	for (size_t j = 0; j < keys.size(); j++)
	{
		int n = N - decoration_pairwise_probs_pu[keys[j]][QPair<int, int>(0, 0)]; // 不计算都不出现的情况
		for (size_t k = 0; k < decoration_pairwise_probs_pu[keys[j]].keys().size(); k++)
		{
			// cluster index pair
			auto key = decoration_pairwise_probs_pu[keys[j]].keys()[k];
			if (key == QPair<int, int>(0, 0))
			{
				decoration_pairwise_probs_pu[keys[j]][key] = 1; // 设都不出现的情况为常数
			}
			decoration_pairwise_probs_pu[keys[j]][key] = (decoration_pairwise_probs_pu[keys[j]][key] + 0.1) / (n + 1);
		}
	}
}

void ProbLearning::CalculateDecorationProb()
{
	m_decoration_types = m_para->DecorationTypes;
	QMap<DecorationType, int> neg_decoration_occurrence; // 记录每个小物件在负样本中出现的次数
	QMap<DecorationType, int> pos_decoration_occurrence; // 记录每个小物件在正样本中出现的次数
	decoration_support_probs.clear(); // 小物体出现在大家具的概率
	for (size_t i = 0; i < m_decoration_types.size(); i++)
	{
		neg_decoration_occurrence[m_decoration_types[i]] = 0;
		pos_decoration_occurrence[m_decoration_types[i]] = 0;
		decoration_probs[m_decoration_types[i]] = 0.0;
	}
	QVector<ImageDecorationType> neg_decorations = m_decorations[0];
	QVector<ImageDecorationType> pos_decorations = m_decorations[1];

	// 第i张图片
	for (size_t i = 0; i < neg_decorations.size(); i++)
	{		
		// 第i张图片的第j个标注
		for (size_t j = 0; j < neg_decorations[i].size(); j++)
		{
			if (neg_decoration_occurrence.contains(neg_decorations[i][j].first)) // DecorationType
			{				
				neg_decoration_occurrence[neg_decorations[i][j].first]++;
				// 添加到小物件和大家具的关系中
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
				// 添加到小物件和大家具的关系中
				addToDecorationSupportProb(pos_decorations[i][j]);
			}
		}
	}

	// 把小物件出现在大家具上转换成概率
	QMapIterator<DecorationType,QMap<FurnitureType,double>> it(decoration_support_probs);
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
	
	// calculate mutual information
	for (size_t i = 0; i < m_decoration_types.size(); i++)
	{
		double A = pos_decoration_occurrence[m_decoration_types[i]];
		double B = neg_decoration_occurrence[m_decoration_types[i]];
		double C = pos_decorations.size() - A;
		double D = neg_decorations.size() - B;
		double N = pos_decorations.size() + neg_decorations.size();
		double MI = log((A*N + 0.01) / ((A + C)*(A + B) + 0.01));
		//double CHI = N*(A*D - C*B)*(A*D - C*B) / ((A + C)*(B + D)*(A + B)*(C + D));
		// only use mutual information
		//decoration_probs[m_decoration_types[i]] = MI;

		// only use CHI
		//decoration_probs[m_decoration_types[i]] = CHI;
		
		double frenquency = (A + B + 0.1) / N;
		// multiply frequency
		//decoration_probs[m_decoration_types[i]] = 1 / (1 + exp(MI));
		double score = 0.0;
		switch (m_pu_type)
		{
		case Prevalence:
			score = frenquency;
			break;
		case Uniqueness:
			score = 1 / (1 + exp(-MI));
			break;
		case PU:
			score = 1 / (1 + exp(-MI))*frenquency;
			break;
		default:
			break;
		}
		decoration_probs[m_decoration_types[i]] = score;		
	}	

	sorted_decoration_types.clear();
	auto keys = decoration_probs.keys();
	for (size_t i = 0; i < keys.size(); i++)
	{
		sorted_decoration_types.push_back(QPair<DecorationType, double>(keys[i], decoration_probs[keys[i]]));
	}
	qSort(sorted_decoration_types.begin(), sorted_decoration_types.end(), Utility::QPairSecondComparer());	
}

void ProbLearning::addToDecorationSupportProb(DecorationLabelType cur_label)
{
	// 统计小物件出现在大物体上的概率	
	if (decoration_support_probs.contains(cur_label.first)) // 已有小物体的key
	{
		// 已有大物体的key
		if (decoration_support_probs[cur_label.first].contains(cur_label.second.first))
		{
			decoration_support_probs[cur_label.first][cur_label.second.first]++;
		}
		else // 新加大物体的key
		{
			decoration_support_probs[cur_label.first][cur_label.second.first] = 1.0;
		}
	}
	else
	{
		QMap<FurnitureType, double> map;
		map[cur_label.second.first] = 1.0;
		decoration_support_probs[cur_label.first] = map;
	}
}

void ProbLearning::SimulatedAnnealingNew()
{
	srand(time(NULL));
	
	//22
	QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	int n = current_furniture_models.size();
	QVector<FurnitureType> types;
	for (size_t i = 0; i < n; i++)
	{
		types.push_back(current_furniture_models[i]->Type);
	}

	// aim: to get QMap<FurnitureType, ColorPalette*>

	// 0. initialize, send random clusterIndex to furniture
	furniture_color_indices.clear();
	for (size_t i = 0; i < n; i++)
	{
		if (furniture_color_clusters[types[i]].keys().size() > 0)
		{
			int index = rand() % furniture_color_clusters[types[i]].keys().size();
			furniture_color_indices[types[i]] = furniture_color_clusters[types[i]].keys()[index];
		}
	}

	decoration_presence.clear();
	auto deco_types = m_para->DecorationTypes;
	for (size_t i = 0; i < deco_types.size(); i++)
	{
		if (!decoration_presence.contains(deco_types[i]))
			decoration_presence[deco_types[i]] = rand() % 2;
	}


	// 1. iterate
	double F = GetScoreAll(furniture_color_indices, decoration_presence);
	double Fold = F;
	double min_F = INT_MAX;
	double beta = 5.0;
	QMap<FurnitureType, ClusterIndex> min_energy_result;

	int k = 0;
	double T0 = -log(0.000000001);
	double deltaT = T0 / (10 * pow(n, 2));
	int max_k = (int)(T0 / deltaT);
	QFile file("./simulated_anealing.txt");
	QTextStream txtOutput(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		std::cout << "can not open simulated_anealing.txt" << std::endl;
		return;
	}
	//int n_converg = 0;
	while (k++ < max_k)
	{
		Fold = F;
		QMap<FurnitureType, ClusterIndex> tmpcolorconfig = furniture_color_indices;
		QMap<DecorationType, int> tmpdecorationconfig = decoration_presence;
		int option = rand() % 2;
		if (option == 0) // change furniture color cluster
		{
			tmpcolorconfig = ChangeFurnitureColor(furniture_color_indices);
		}
		else // change decoraiton presence
		{
			tmpdecorationconfig = ChangeDecorationPresence(decoration_presence);
		}
		
		F = GetScoreAll(tmpcolorconfig, tmpdecorationconfig);
		double accept_rate = GetAcceptRate(F, Fold, T0, deltaT, k);
		//double accept_rate = qMin(1.0, exp(-beta*F) / exp(-beta*Fold));

		if ((static_cast<double>(rand()) / (RAND_MAX)) < accept_rate) // accetpted
		{		
			furniture_color_indices = tmpcolorconfig;
			decoration_presence = tmpdecorationconfig;
		}
		else // if not, keep F unchanged
		{
			//n_converg++;
			F = Fold;
			//k--;
		}
		if (k % 10 == 0)
		{
			txtOutput << F << "\n";
			std::cout << "SimulatedAnealing: " << k << " F: " << F << std::endl;
		}
		/*if (n_converg >= 100)
		{
		break;
		}*/
	}

	txtOutput << "final F = " << F << "\n";
	std::cout << "final F: " << F << std::endl;
	
	txtOutput << "Furniture Colors\n";
	QMapIterator<QString, int> it(furniture_color_indices);
	while (it.hasNext())
	{
		it.next();
		txtOutput << it.key() << " " << it.value() << "\n";
	}

	txtOutput << "Decoraitons\n";
	QMapIterator<QString, int> it2(decoration_presence);
	while (it2.hasNext())
	{
		it2.next();
		txtOutput << it2.key() << " " << it2.value() << "\n";
	}

	file.close();
}

void ProbLearning::SimulatedAnnealing()
{
	srand(time(NULL));
	double lambda1 = 1 / 3.0, lambda2 = 1 / 3.0, lambda3 = 1 / 3.0;

	//
	//22
	QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	int n = current_furniture_models.size();
	QVector<FurnitureType> types;
	for (size_t i = 0; i < n; i++)
	{
		types.push_back(current_furniture_models[i]->Type);
	}

	// aim: to get QMap<FurnitureType, ColorPalette*>

	// 0. initialize, send random clusterIndex to furniture
	furniture_color_indices.clear();
	for (size_t i = 0; i < n; i++)
	{
		if (furniture_color_clusters[types[i]].keys().size() > 0)
		{
			int index = rand() % furniture_color_clusters[types[i]].keys().size();
			furniture_color_indices[types[i]] = furniture_color_clusters[types[i]].keys()[index];
		}
	}

	// 1. iterate	
	double F = GetScore(furniture_color_indices);
	double Fold = F;
	double min_F = INT_MAX;
	double beta = 5.0;
	QMap<FurnitureType, ClusterIndex> min_energy_result;
		
	int k = 0;
	double T0 = -log(0.000000001);
	double deltaT = T0 / (500 * pow(n, 2));
	int max_k = (int)(T0 / deltaT);
	QFile file("./simulated_anealing.txt");
	QTextStream txtOutput(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		std::cout << "can not open simulated_anealing.txt" << std::endl;
		return;
	}
	//int n_converg = 0;
	while (k++ < max_k)
	{
		Fold = F;
		QMap<FurnitureType,ClusterIndex> tmpcolorconfig = ChangeFurnitureColor(furniture_color_indices);
		F = GetScore(tmpcolorconfig);
		double accept_rate = GetAcceptRate(F, Fold, T0, deltaT, k);
		//double accept_rate = qMin(1.0, exp(-beta*F) / exp(-beta*Fold));

		if ((static_cast<double>(rand()) / (RAND_MAX)) < accept_rate) // accetpted
		{
			//n_converg = 0;
			furniture_color_indices = tmpcolorconfig;
		}
		else // if not, keep F unchanged
		{
			//n_converg++;
			F = Fold;
			//k--;
		}		
		if (k%10 == 0)
		{
			txtOutput << F << "\n";
			std::cout << "SimulatedAnealing: " <<  k << " F: " << F << std::endl;
		}
		/*if (n_converg >= 100)
		{
			break;
		}*/
	}

	txtOutput << "final F = " << F << "\n";
	std::cout << "final F: " << F << std::endl;
	QMapIterator<QString, int> it(furniture_color_indices);
	while (it.hasNext())
	{
		it.next();
		txtOutput << it.key() << " " << it.value() << "\n";
	}
	file.close();

	// convert cluster index to colorpalette
}

void ProbLearning::MCMCSamplingNew()
{
	srand(time(NULL));

	//
	//22
	QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	int n = current_furniture_models.size();
	QVector<FurnitureType> types;
	for (size_t i = 0; i < n; i++)
	{
		types.push_back(current_furniture_models[i]->Type);
	}

	// aim: to get QMap<FurnitureType, ColorPalette*>

	// 0. initialize, send random clusterIndex to furniture
	furniture_color_indices.clear();
	for (size_t i = 0; i < n; i++)
	{
		if (furniture_color_clusters[types[i]].keys().size() > 0)
		{
			int index = rand() % furniture_color_clusters[types[i]].keys().size();
			furniture_color_indices[types[i]] = furniture_color_clusters[types[i]].keys()[index];
		}
	}

	decoration_presence.clear();
	auto deco_types = m_para->DecorationTypes;
	for (size_t i = 0; i < deco_types.size(); i++)
	{
		if (!decoration_presence.contains(deco_types[i]))
			decoration_presence[deco_types[i]] = rand() % 2;
	}


	QMap<QString, ClusterIndex> old_furniture_color_indices;
	QMap<QString, int> old_decoration_indices;
	// 1. iterate	
	double F = GetScoreAll(furniture_color_indices,decoration_presence);
	double Fold = F;
	//double min_F = INT_MAX;
	double beta = 5.0;
	QMap<FurnitureType, ClusterIndex> min_energy_color_config;
	QMap<DecorationType, int> min_energy_decoration_config;
	QList<QPair<QPair<QMap<QString, ClusterIndex>,QMap<QString, int>>, double>> all_mcmc_results;
	int k = 0;
	//double T0 = -log(0.000000001);
	//double deltaT = T0 / (100 * pow(n, 2));
	int max_k = 10000;
	QFile file("./MCMC.txt");
	QTextStream txtOutput(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		std::cout << "can not open MCMC.txt" << std::endl;
		return;
	}
	all_mcmc_results.clear();
	//int n_converg = 0;
	while (k++ < max_k)
	{
		Fold = F;
		old_furniture_color_indices = QMap<QString, ClusterIndex>(furniture_color_indices);
		old_decoration_indices = QMap<QString, int>(decoration_presence);
		furniture_color_indices = ChangeFurnitureColor(furniture_color_indices);
		decoration_presence = ChangeDecorationPresence(decoration_presence);
		F = GetScoreAll(furniture_color_indices, decoration_presence);
		//double accept_rate = GetAcceptRate(F, Fold, T0, deltaT, k);
		double accept_rate = qMin(1.0, exp(-beta*F) / exp(-beta*Fold));
		if ((static_cast<double>(rand()) / (RAND_MAX)) < accept_rate) // accetpted
		{
			if (accept_rate < 1)
			{
				int a = 0;
			}
			Fold = F;
			//all_results.push_back(QPair<QMap<QString, ClusterIndex>, double>(QMap<QString, ClusterIndex>(furniture_color_indices), F));
			QPair<QMap<QString, ClusterIndex>, QMap<QString, int>> pair(furniture_color_indices, decoration_presence);
			all_mcmc_results.push_back(QPair<QPair<QMap<QString, ClusterIndex>, QMap<QString, int>>, double>(pair, F));
			//n_converg = 0;	

		}
		else // if not, keep F unchanged
		{
			//n_converg++;
			furniture_color_indices = old_furniture_color_indices;
			decoration_presence = old_decoration_indices;
			//k--;
		}
		if (k % 10 == 0)
		{
			std::cout << "MCMC: " << k << " F: " << F << std::endl;
		}
	}
	qSort(all_mcmc_results.begin(), all_mcmc_results.end(), Utility::QPairSecondComparerAscending());
	int all_size = all_mcmc_results.size();
	furniture_color_indices = all_mcmc_results[0].first.first;
	decoration_presence = all_mcmc_results[0].first.second;
	txtOutput << all_mcmc_results[0].second << "\n";
	//txtOutput << "final F = " << F << "\n";
	//std::cout << "final F: " << F << std::endl;
	QMapIterator<QString, int> it(furniture_color_indices);
	while (it.hasNext())
	{
		it.next();
		txtOutput << it.key() << " " << it.value() << "\n";
	}
	file.close();
	// convert cluster index to colorpalette
}

void ProbLearning::MCMCSampling()
{
	srand(time(NULL));
	double lambda1 = 1 / 3.0, lambda2 = 1 / 3.0, lambda3 = 1 / 3.0;

	//
	//22
	QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	int n = current_furniture_models.size();
	QVector<FurnitureType> types;
	for (size_t i = 0; i < n; i++)
	{
		types.push_back(current_furniture_models[i]->Type);
	}

	// aim: to get QMap<FurnitureType, ColorPalette*>

	// 0. initialize, send random clusterIndex to furniture
	furniture_color_indices.clear();
	for (size_t i = 0; i < n; i++)
	{
		if (furniture_color_clusters[types[i]].keys().size() > 0)
		{
			int index = rand() % furniture_color_clusters[types[i]].keys().size();
			furniture_color_indices[types[i]] = furniture_color_clusters[types[i]].keys()[index];
		}
	}

	QMap<QString, ClusterIndex> old_furniture_color_indices;
	// 1. iterate	
	double F = GetScore(furniture_color_indices);
	double Fold = F;
	//double min_F = INT_MAX;
	double beta = 5.0;
	QMap<FurnitureType, ClusterIndex> min_energy_result;
	QList<QPair<QMap<QString, ClusterIndex>, double>> all_results;
	int k = 0;
	//double T0 = -log(0.000000001);
	//double deltaT = T0 / (100 * pow(n, 2));
	int max_k = 112500;
	QFile file("./MCMC.txt");
	QTextStream txtOutput(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		std::cout << "can not open MCMC.txt" << std::endl;
		return;
	}

	//int n_converg = 0;
	while (k++ < max_k)
	{
		Fold = F;
		old_furniture_color_indices = QMap<QString,ClusterIndex>(furniture_color_indices);
		furniture_color_indices = ChangeFurnitureColor(furniture_color_indices);
		F = GetScore(furniture_color_indices);
		//double accept_rate = GetAcceptRate(F, Fold, T0, deltaT, k);
		double accept_rate = qMin(1.0, exp(-beta*F) / exp(-beta*Fold));

		if ((static_cast<double>(rand()) / (RAND_MAX)) < accept_rate) // accetpted
		{
			if (accept_rate < 1)
			{
				int a = 0;
			}
			Fold = F;
			all_results.push_back(QPair<QMap<QString, ClusterIndex>, double>(QMap<QString, ClusterIndex>(furniture_color_indices), F));
			//n_converg = 0;	
			
		}
		else // if not, keep F unchanged
		{
			//n_converg++;
			furniture_color_indices = old_furniture_color_indices;
			//k--;
		}
		if (k % 10 == 0)
		{			
			std::cout << "MCMC: " << k << " F: " << F << std::endl;
		}		
	}
	qSort(all_results.begin(), all_results.end(), Utility::QPairSecondComparerAscending());
	furniture_color_indices = all_results[0].first;
	txtOutput << all_results[0].second << "\n";
	//txtOutput << "final F = " << F << "\n";
	//std::cout << "final F: " << F << std::endl;
	QMapIterator<QString, int> it(furniture_color_indices);
	while (it.hasNext())
	{
		it.next();
		txtOutput << it.key() << " " << it.value() << "\n";
	}
	file.close();
	// convert cluster index to colorpalette
}

void ProbLearning::MCMCMinimumCoverSelect()
{
	srand(time(NULL));

	//
	//22
	QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	int n = current_furniture_models.size();
	QVector<FurnitureType> types;
	for (size_t i = 0; i < n; i++)
	{
		types.push_back(current_furniture_models[i]->Type);
	}

	// aim: to get QMap<FurnitureType, ColorPalette*>

	// 0. initialize, send random clusterIndex to furniture
	furniture_color_indices.clear();
	for (size_t i = 0; i < n; i++)
	{
		if (furniture_color_clusters[types[i]].keys().size() > 0)
		{
			int index = rand() % furniture_color_clusters[types[i]].keys().size();
			furniture_color_indices[types[i]] = furniture_color_clusters[types[i]].keys()[index];
		}
	}

	decoration_presence.clear();
	auto deco_types = m_para->DecorationTypes;
	for (size_t i = 0; i < deco_types.size(); i++)
	{
		if (!decoration_presence.contains(deco_types[i]))
			decoration_presence[deco_types[i]] = rand() % 2;
	}


	QMap<QString, ClusterIndex> old_furniture_color_indices;
	QMap<QString, int> old_decoration_indices;
	// 1. iterate	
	double F = GetScoreAll(furniture_color_indices, decoration_presence);
	double Fold = F;
	//double min_F = INT_MAX;
	double beta = 5.0;
	QMap<FurnitureType, ClusterIndex> min_energy_color_config;
	QMap<DecorationType, int> min_energy_decoration_config;	
	QList<vector<double>> scores;
	int k = 0;
	//double T0 = -log(0.000000001);
	//double deltaT = T0 / (100 * pow(n, 2));
	int max_k = 10000;
	QFile file("./MCMC.txt");
	QTextStream txtOutput(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		std::cout << "can not open MCMC.txt" << std::endl;
		return;
	}
	all_mcmc_results.clear();
	all_mcmc_ordered_results.clear();
	//int n_converg = 0;
	while (k++ < max_k)
	{
		Fold = F;
		vector<double> scorevector;
		old_furniture_color_indices = QMap<QString, ClusterIndex>(furniture_color_indices);
		old_decoration_indices = QMap<QString, int>(decoration_presence);
		furniture_color_indices = ChangeFurnitureColor(furniture_color_indices);
		decoration_presence = ChangeDecorationPresence(decoration_presence);
		F = GetScoreAll(furniture_color_indices, decoration_presence);		
		//double accept_rate = GetAcceptRate(F, Fold, T0, deltaT, k);
		double accept_rate = qMin(1.0, exp(-beta*F) / exp(-beta*Fold));
		
		if ((static_cast<double>(rand()) / (RAND_MAX)) < accept_rate) // accetpted
		{
			if (accept_rate < 1)
			{
				int a = 0;
			}
			Fold = F;
			scorevector = GetScoreVector(furniture_color_indices, decoration_presence);
			scores.push_back(scorevector);
			//all_results.push_back(QPair<QMap<QString, ClusterIndex>, double>(QMap<QString, ClusterIndex>(furniture_color_indices), F));
			QPair<QMap<QString, ClusterIndex>, QMap<QString, int>> pair(furniture_color_indices, decoration_presence);
			all_mcmc_results.push_back(QPair<QPair<QMap<QString, ClusterIndex>, QMap<QString, int>>, double>(pair, F));
			all_mcmc_ordered_results.push_back(QPair<QPair<QMap<QString, ClusterIndex>, QMap<QString, int>>, double>(pair, F));
			//n_converg = 0;	

		}
		else // if not, keep F unchanged
		{
			//n_converg++;
			furniture_color_indices = old_furniture_color_indices;
			decoration_presence = old_decoration_indices;
			//k--;
		}
		if (k % 10 == 0)
		{
			std::cout << "MCMC: " << k << " F: " << F << std::endl;
		}
	}

	int col = scores.size();
	int row = scores[0].size();
	
	double ** matrix = new double*[row];
	for (size_t i = 0; i < row; i++)
	{
		matrix[i] = new double[col];
	}
	for (size_t i = 0; i < row; i++)
	{
		for (size_t j = 0; j < col; j++)
		{
			matrix[i][j] = scores[j][i];
		}
	}
	selected_indices = MinimizeCoverageSelection::GetRepresentativeNodes(matrix, row, col, 5);
	for (size_t i = 0; i < row; i++)
	{
		delete[]matrix[i];
	}
	delete[]matrix;
	qSort(all_mcmc_ordered_results.begin(), all_mcmc_ordered_results.end(), Utility::QPairSecondComparerAscending());
	// int all_size = all_mcmc_results.size();
	if (Parameter::GetParameterInstance()->SelectSampleMethodType == 0) //MCMC
	{
		furniture_color_indices = all_mcmc_ordered_results[0].first.first;
		decoration_presence = all_mcmc_ordered_results[0].first.second;
		txtOutput << all_mcmc_ordered_results[0].second << "\n";
	}
	else if(Parameter::GetParameterInstance()->SelectSampleMethodType == 1) // submodular
	{
		furniture_color_indices = all_mcmc_results[selected_indices[0]].first.first;
		decoration_presence = all_mcmc_results[selected_indices[0]].first.second;
		txtOutput << all_mcmc_results[selected_indices[0]].second << "\n";
	}	
	cout << "F = " << this->GetFAll() << "\n";
	//txtOutput << "final F = " << F << "\n";
	//std::cout << "final F: " << F << std::endl;
	QMapIterator<QString, int> it(furniture_color_indices);
	while (it.hasNext())
	{
		it.next();
		txtOutput << it.key() << " " << it.value() << "\n";
	}
	file.close();
	// convert cluster index to colorpalette
}

void ProbLearning::ConvexMaxProduct()
{
	QFile file("./CMP.txt");
	QTextStream txtOutput(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		std::cout << "can not open CMP.txt" << std::endl;
		return;
	}
	QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	int n = current_furniture_models.size();
	QVector<FurnitureType> types;
	
	for (size_t i = 0; i < n; i++)
	{
		types.push_back(current_furniture_models[i]->Type);
	}

	int var_num = types.size(); // number of variables
	int label_num = m_para->FurnitureClusterNum;
	core::FactorGraph fg;	
	auto vcid = fg.addVarCategory(label_num, 1.0);

	auto furniture_unary_probs = furniture_color_probs;
	for (size_t f = 0; f < var_num; f++)
	{
		auto fcid = fg.addFactorCategory(
			
			[f, furniture_unary_probs,types,n](const std::vector<int> &labels) -> double {
			double e = -0.5*log(furniture_unary_probs[types[f]][labels[0]] + 0.000000001) / n;
			/*double e = -0.5*log(furniture_unary_probs[types[f]][labels[0]] + 0.000000001);*/
			assert(e >= 0);
			return e;
		},
			1.0);

		auto vh = fg.addVar(vcid);
		auto fh = fg.addFactor(fcid, { vh });
	}

	//auto furniture_binary_probs = furniture_pairwise_color_probs;

	//for (int f1 = 0; f1 < var_num; f1++)
	//{
	//	for (int f2 = 0; f2 < var_num; f2++)
	//	{
	//		if (furniture_binary_probs.contains(QPair<FurnitureType, FurnitureType>(types[f1], types[f2])))
	//		{
	//			auto fcid = fg.addFactorCategory(
	//				[f1, f2, furniture_binary_probs, types, n](const std::vector<int> &labels) -> double {
	//					double e = -0.5*log(furniture_binary_probs[QPair<FurnitureType, FurnitureType>(types[f1], types[f2])]
	//						[QPair<ClusterIndex, ClusterIndex>(labels[0], labels[1])] + 0.000000001)*2.0 / (n*(n - 1));
	//				/*double e = -0.5*log(furniture_binary_probs[QPair<FurnitureType, FurnitureType>(types[f1], types[f2])]
	//					[QPair<ClusterIndex, ClusterIndex>(labels[0], labels[1])] + 0.000000001);*/
	//					assert(e >= 0);
	//					return e;

	//			},
	//				1.0);
	//			auto fh = fg.addFactor(fcid, { f1,f2 });
	//		}				
	//	}	
	//}	

	auto results = fg.solve(100, 1, [](int epoch, double energy) {
		std::cout << epoch << "energy: " << energy << std::endl;	
		return true;
	});

	furniture_color_indices.clear();
	for (size_t t = 0; t < types.size(); t++)
	{
		furniture_color_indices[types[t]] = results[t];
	}
	double F = GetScore(furniture_color_indices);

	txtOutput << "F = " << F << "\n";
	file.close();
//	ASSERT(results[vh] == 1);

}

void ProbLearning::ConvexMaxProductDecorations()
{
	//QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	
	//QVector<FurnitureType> types;
	auto types = m_para->DecorationTypes;
	int n = m_decoration_types.size();
	int var_num = n;	
	int label_num = 2; // 1 for presence, 0 for absence
	core::FactorGraph fg;
	auto vcid = fg.addVarCategory(label_num, 1.0);

	auto decoration_unary_prob = decoration_probs_pu;
	for (size_t d = 0; d < var_num; d++)
	{
		auto fcid = fg.addFactorCategory(

			[d, decoration_unary_prob, types, n](const std::vector<int> &labels) -> double {
			double e = -0.5*log(decoration_unary_prob[types[d]][labels[0]] + 0.000000001);
			//double e = 0.1;
			assert(e >= 0);
			return e;
		},
			1.0);
		auto vh = fg.addVar(vcid);
		auto fh = fg.addFactor(fcid, { vh });
	}

	auto decoration_binary_probs = decoration_pairwise_probs_pu;

	for (int d1 = 0; d1 < var_num; d1++)
	{
		for (int d2 = 0; d2 < var_num; d2++)
		{
			if (decoration_binary_probs.contains(QPair<DecorationType, DecorationType>(types[d1], types[d2])))
			{
				auto fcid = fg.addFactorCategory(
					[d1, d2, decoration_binary_probs, types, n](const std::vector<int> &labels) -> double {
					double e = -0.5*log(decoration_binary_probs[QPair<DecorationType, DecorationType>(types[d1], types[d2])]
						[QPair<int, int>(labels[0], labels[1])] + 0.000000001);
					assert(e >= 0);
					return e;

				},
					1.0);
				auto fh = fg.addFactor(fcid, { d1,d2 });
			}
		}
	}

	QList<QPair<std::vector<int>,double>> all_results;
	auto results = fg.solve(50, 1, [&all_results](int epoch, double energy, double denergy,
		const std::vector<int> &cur_best_var_labels) {
		all_results.push_back(QPair<std::vector<int>, double>(cur_best_var_labels, energy));
		std::cout << epoch << " energy: " << energy << std::endl;
		return true;
	});

	qSort(all_results.begin(), all_results.end(), Utility::QPairSecondComparerAscending());

	QVector<QMap<DecorationType, int>> all_decoration_presence;
	for (size_t i = 0; i < all_results.size(); i++)
	{
		QMap<DecorationType, int> presence;
		for (size_t t = 0; t < types.size(); t++)
		{
			presence[types[t]] = all_results[i].first[t];
		}
		all_decoration_presence.push_back(presence);
	}

	decoration_presence.clear();
	for (size_t t = 0; t < types.size(); t++)
	{
		decoration_presence[types[t]] = results[t];
	}
}

void ProbLearning::BruteForce()
{
	srand(time(NULL));

	//
	//22
	QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	int n = current_furniture_models.size();
	QVector<FurnitureType> types;
	for (size_t i = 0; i < n; i++)
	{
		types.push_back(current_furniture_models[i]->Type);
	}

	//QList<QMap<QString, int>> all_indices;
	QList<QPair<QMap<QString, int>, double>> index_score;
	int clusternum = m_para->FurnitureClusterNum;
	unsigned int totalcomp = pow(clusternum, types.size());
	std::cout << "total times: " << totalcomp << std::endl;
	for (size_t i = 0; i < totalcomp; i++)
	{
		QMap<QString, int> map;
		for (size_t t = 0; t < types.size(); t++)
		{
			map[types[t]] = (int)(i / (pow(clusternum, t))) % clusternum;
		}		
		double F = GetScore(map);
		index_score.push_back(QPair<QMap<QString, int>, double>(map, F));

		if (i % 1000 == 0)
		{
			std::cout << "iterate " << i << std::endl;
		}
	}

	qSort(index_score.begin(), index_score.end(), Utility::QPairSecondComparerAscending());

	furniture_color_indices = index_score[0].first;


	QFile file("./bruteforce.txt");
	QTextStream txtOutput(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		std::cout << "can not open simulated_anealing.txt" << std::endl;
		return;
	}
	txtOutput << "final F = " << index_score[0].second << "\n";
	QMapIterator<QString, int> it(furniture_color_indices);
	while (it.hasNext())
	{
		it.next();
		txtOutput << it.key() << " " << it.value() << "\n";
	}
	file.close();
	
}

double ProbLearning::GetScore(QMap<QString, ClusterIndex> furniture_colors)
{
	double score = 0.0;
	switch (m_energy_type)
	{
	case F1:
		score = GetScoreF1(furniture_colors);
		break;
	case F2:
		score = GetScoreF2(furniture_colors);
		break;
	case F1F2:
		score = 0.5*GetScoreF1(furniture_colors) + 0.5*GetScoreF2(furniture_colors);
		break;
	default:
		break;
	}
	return score;
}

double ProbLearning::GetScoreF1(QMap<FurnitureType, ClusterIndex> furniture_colors)
{
	double score = 0.0;
	QMapIterator<FurnitureType, ClusterIndex> it(furniture_colors);
	while (it.hasNext()) 
	{
		it.next();
		score += log(furniture_color_probs[it.key()][it.value()] + 0.000000001);
	}
	score = - 1.0 / furniture_colors.size() * score;	
	return score;
}

double ProbLearning::GetScoreF2(QMap<FurnitureType, ClusterIndex> furniture_colors)
{
	double score = 0.0;
	QList<FurnitureType> all_types = furniture_colors.keys();
	int n = all_types.size();
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			int ci = furniture_colors[all_types[i]];
			int cj = furniture_colors[all_types[j]];
			if (furniture_pairwise_color_probs.contains(QPair<FurnitureType, FurnitureType>(all_types[i], all_types[j])))
			{
				score += log(furniture_pairwise_color_probs[QPair<FurnitureType, FurnitureType>(all_types[i], all_types[j])]
					[QPair<ClusterIndex, ClusterIndex>(ci, cj)] + 0.000000001);
			}			
		}		
	}
	score = -2.0 / (n*(n - 1))*score;
	return score;
}

double ProbLearning::GetAcceptRate(double F, double Fold, double T0, double deltaT, int k)
{
	return qMin(1.0, exp(-(F - Fold) / (T0 - deltaT*k)));
}

QMap<QString, ClusterIndex> ProbLearning::ChangeFurnitureColor(QMap<QString, ClusterIndex> furniture_colors)
{
	QMap<QString, ClusterIndex> map;
	QMapIterator<FurnitureType, ClusterIndex> it(furniture_colors);
	while (it.hasNext())
	{
		it.next();
		map[it.key()] = it.value();
	}
	// randomly choose a furniture
	int index = rand() % map.size();
	int new_cluster = 0;
	FurnitureType furniture_type = map.keys()[index];
	// choose a new cluster index for this furniture
	while (1)
	{
		new_cluster = rand() % furniture_color_clusters[furniture_type].keys().size();
		if (new_cluster != furniture_colors[furniture_type])
			break;		
	}
	map[furniture_type] = new_cluster;
	return map;
}

QMap<QString, int> ProbLearning::ChangeDecorationPresence(QMap<QString, int> deco_presence)
{
	QMap<QString, int> map;
	QMapIterator<DecorationType, ClusterIndex> it(deco_presence);
	while (it.hasNext())
	{
		it.next();
		map[it.key()] = it.value();
	}
	// randomly choose a decoration
	int index = rand() % map.size();	
	DecorationType decoration_type = map.keys()[index];
	// flip
	map[decoration_type] = deco_presence[decoration_type] == 0 ? 1 : 0;	
	return map;
}

double ProbLearning::GetUnaryScore(QMap<QString, int> labels, QMap<QString, QMap<int, double>> data)
{
	// data 可能是furniture_color_probs 或 decoration_probs_pu
	double score = 0.0;
	// 如果输入是家具，则是FurnitureType, ClusterIndex;
	// 如果输入是小物体，则是DecorationType, int(1/0)
	QMapIterator<QString, int> it(labels);
	while (it.hasNext())
	{
		it.next();
		score += log(data[it.key()][it.value()] + Epsilon);
	}
	score = -1.0 / labels.size() * score;
	return score;
}

double ProbLearning::GetBinaryScore(QMap<QString, int> labels1, QMap<QString, int> labels2,
	QMap<QPair<FurnitureType, FurnitureType>, QMap<QPair<ClusterIndex, ClusterIndex>, double>> data)
{
	double score = 0.0;
	QList<QString> all_types1 = labels1.keys();
	QList<QString> all_types2 = labels2.keys();
	int n1 = all_types1.size();
	int n2 = all_types2.size();
	int totalsize = 0;
	for (size_t i = 0; i < n1; i++)
	{
		for (size_t j = 0; j < n2; j++)
		{
			int ci = labels1[all_types1[i]];
			int cj = labels2[all_types2[j]];
			if (data.contains(QPair<QString, QString>(all_types1[i], all_types2[j])))
			{
				score += log(data[QPair<QString, QString>(all_types1[i], all_types2[j])]
					[QPair<int, int>(ci, cj)] + Epsilon);
				totalsize++;
			}
		}
	}
	//score = totalsize > 0 ? -2.0 / (totalsize*(totalsize - 1))*score : 0;
	score = totalsize > 0 ? -score / totalsize : 0;
	return score;
}

double ProbLearning::GetDecorationNumberCost(QMap<QString, int> decolables)
{
	QMapIterator<QString, int> it(decolables);
	int n = 0;
	while (it.hasNext())
	{
		it.next();
		if (it.value() == 1)
		{
			n++;
		}
	}
	double diff = n - m_para->DecorationNumber;
	// mean = 0, variance^2 = 9
	double guassian = 1 / (3*sqrt(2 * 3.1415926))*exp(-(diff*diff) / (18));
	double neglog = -log(guassian);
	return -log(guassian);	
}

double ProbLearning::GetScoreAll(QMap<QString, ClusterIndex> furlabels, QMap<QString, int> decolabels)
{
	double score = 0.2 * GetUnaryScore(furlabels, furniture_color_probs) +
		0.2 * GetUnaryScore(decolabels, decoration_probs_pu) + 
		0.2 * GetBinaryScore(furlabels, furlabels, furniture_pairwise_color_probs) +
		0.2 * GetBinaryScore(decolabels, decolabels, decoration_pairwise_probs_pu) +
		0.2 * GetBinaryScore(furlabels, decolabels, furniture_decoration_probs_pu) +
		0.2 * GetDecorationNumberCost(decolabels)
		;
	
	return score;
}

QVector<double> ProbLearning::GetUnaryScoreVector(QMap<QString, int> labels, QMap<QString, QMap<int, double>> data)
{
	QVector<double> unaryscores;
	// data 可能是furniture_color_probs 或 decoration_probs_pu
	double score = 0.0;
	// 如果输入是家具，则是FurnitureType, ClusterIndex;
	// 如果输入是小物体，则是DecorationType, int(1/0)
	QMapIterator<QString, int> it(labels);
	while (it.hasNext())
	{		
		it.next();
		score = -1.0 / labels.size() * log(data[it.key()][it.value()] + Epsilon);
		unaryscores.push_back(score);
		//score += log(data[it.key()][it.value()] + Epsilon);
	}
	return unaryscores;
}

QVector<double> ProbLearning::GetBinaryScoreVector(QMap<QString, int> labels1, QMap<QString, int> labels2, QMap<QPair<QString, QString>, QMap<QPair<ClusterIndex, ClusterIndex>, double>> data)
{
	QVector<double> binaryscores;	
	QList<QString> all_types1 = labels1.keys();
	QList<QString> all_types2 = labels2.keys();
	int n1 = all_types1.size();
	int n2 = all_types2.size();	
	for (size_t i = 0; i < n1; i++)
	{
		for (size_t j = 0; j < n2; j++)
		{
			int ci = labels1[all_types1[i]];
			int cj = labels2[all_types2[j]];
			if (data.contains(QPair<QString, QString>(all_types1[i], all_types2[j])))
			{
				binaryscores.push_back(-log(data[QPair<QString, QString>(all_types1[i], all_types2[j])]
					[QPair<int, int>(ci, cj)] + Epsilon));				
			}
		}
	}
	int totalsize = binaryscores.size();
	for (size_t i = 0; i < totalsize; i++)
	{
		binaryscores[i] /= totalsize;
	}
	return binaryscores;
}

vector<double> ProbLearning::GetScoreVector(QMap<QString, ClusterIndex> furlabels, QMap<QString, int> decolabels)
{	 
	vector<double> all;
	QVector<double> f1vec = GetUnaryScoreVector(furlabels, furniture_color_probs);
	QVector<double> d1vec = GetUnaryScoreVector(decolabels, decoration_probs_pu);
	QVector<double> f1f2vec = GetBinaryScoreVector(furlabels, furlabels, furniture_pairwise_color_probs);
	QVector<double> d1d2vec = GetBinaryScoreVector(decolabels, decolabels, decoration_pairwise_probs_pu);
	QVector<double> f1d1vec = GetBinaryScoreVector(furlabels, decolabels, furniture_decoration_probs_pu);
	all.reserve(1 + f1vec.size() + d1vec.size() + f1f2vec.size() + d1d2vec.size() + f1d1vec.size());	
	all.insert(all.end(), f1vec.begin(), f1vec.end());
	all.insert(all.end(), d1vec.begin(), d1vec.end());
	all.insert(all.end(), f1f2vec.begin(), f1f2vec.end());
	all.insert(all.end(), d1d2vec.begin(), d1d2vec.end());
	all.insert(all.end(), f1d1vec.begin(), f1d1vec.end());
	all.push_back(GetDecorationNumberCost(decolabels));
	return all;
	//	auto f1vec = GetDecorationNumberCost(decolabels);
}

QMap<FurnitureType, ColorPalette*> ProbLearning::GetFurnitureColorPalette(int level = 0)
{
	if (Parameter::GetParameterInstance()->SelectSampleMethodType == 0) // MCMC
	{
		furniture_color_indices = all_mcmc_ordered_results[level%all_mcmc_ordered_results.size()].first.first;
	}
	else if (Parameter::GetParameterInstance()->SelectSampleMethodType == 1) // submodular
	{
		int n = selected_indices.size();
		if (n > 0)
		{
			furniture_color_indices = all_mcmc_results[selected_indices[level%n]].first.first;
		}
	}
	else if (Parameter::GetParameterInstance()->SelectSampleMethodType == 2) // cmp
	{
		// do nothing
	}

	QMap<FurnitureType, ColorPalette*> map;
	QMapIterator<FurnitureType, ClusterIndex> it(furniture_color_indices);
	
	while (it.hasNext())
	{
		it.next();
		// the colorpalette num in this cluster
		//auto all_cp = furniture_color_clusters[it.key()][it.value()];
		auto all_cp = furniture_color_clusters_ordered[it.key()][it.value()];
		if (all_cp.size() == 0)
		{
			qWarning("The %d th cluster for %s is empty", it.value(), it.key().toStdString().c_str());
			continue;
		}
		//int num = all_cp.size();
		//int num = all_cp.size()* 4 / 5 + 1;
		// randomly choose a colorpalette from that cluster
		vector<ColorPalette*> pos_cps;
		for (size_t i = 0; i < all_cp.size(); i++)
		{
			if (all_cp[i]->SampleType == Pos)
			{
				pos_cps.push_back(all_cp[i]);
			}
		}
		
		int num = pos_cps.size();
		ColorPalette* cp;
		if (pos_cps.size() > 0)
		{
			cp = pos_cps[0];
		}			
		if (num > 0) // only select from positive samples
		{
			//cp = pos_cps[rand() % num];
			cp = pos_cps[rand() % num];
			map[it.key()] = cp;
			qInfo("%s: %d", it.key().toStdString().c_str(), it.value());
		}
		else
		{
			cp = all_cp[0];
			map[it.key()] = cp;
			qWarning("The %d th cluster for %s is empty", it.value(), it.key().toStdString().c_str());
		}			
	}	
	return map;
}

QMap<QString, ColorPalette*> ProbLearning::GetFurnitureColorPaletteRandom()
{
	QMap<FurnitureType, ColorPalette*> map;
	if (furniture_color_palettes.size() > 0)
	{
		QMapIterator<FurnitureType, QVector<ColorPalette*>> it(furniture_color_palettes);
		while (it.hasNext())
		{
			it.next();
			// all colorpalette of this furniture type
			auto all_cp = it.value();
			int num = all_cp.size();
			// randomly choose a colorpalette from that cluster
			/*vector<ColorPalette*> pos_cps;
			for (size_t i = 0; i < all_cp.size(); i++)
			{
				if (all_cp[i]->SampleType == Pos)
				{
					pos_cps.push_back(all_cp[i]);
				}
			}
			int num = pos_cps.size();*/
			ColorPalette* cp;
			if (all_cp.size() > 0)
			{
				cp = all_cp[0];
			}
			if (num > 0) // only select from positive samples
			{
				//cp = pos_cps[rand() % num];
				cp = all_cp[rand() % num];
				map[it.key()] = cp;
			}
			else
			{
				qWarning("The %d th cluster for %s is empty", it.value(), it.key().toStdString().c_str());
			}
		}
	}	
	return map;
}

QList<QPair<DecorationType, QList<QPair<FurnitureType, double>>>> ProbLearning::GetDecorationTypesByNumber(int n)
{
	QList<QPair<DecorationType, QList<QPair<FurnitureType, double>>>> list;
	n = n > sorted_decoration_types.size() ? sorted_decoration_types.size() : n;	
	for (size_t i = 0; i < n; i++)
	{
		DecorationType dt = sorted_decoration_types[i].first;
		if (decoration_support_probs.contains(dt))
		{
			QPair<DecorationType, QList<QPair<FurnitureType, double>>> pair;
			pair.first = dt;
			auto innerlist = Utility::QMap2QList(decoration_support_probs[dt]);
			qSort(innerlist.begin(), innerlist.end(), Utility::QPairSecondComparer());
			pair.second = innerlist;
			list.push_back(pair);
		}
	}
	return list;
}

QList<QPair<QString, QList<QPair<QString, double>>>> ProbLearning::GetDecorationTypes(int level = 0)
{
	if (Parameter::GetParameterInstance()->SelectSampleMethodType == 0) // MCMC
	{
		decoration_presence = all_mcmc_ordered_results[level%all_mcmc_ordered_results.size()].first.second;
	}
	else if(Parameter::GetParameterInstance()->SelectSampleMethodType == 1) // submodular
	{
		int n = selected_indices.size();
		if (n > 0)
		{
			decoration_presence = all_mcmc_results[selected_indices[level%n]].first.second;
		}
	}
	
	QList<QPair<DecorationType, QList<QPair<FurnitureType, double>>>> list;
	auto keys = decoration_presence.keys();
	for (size_t i = 0; i < keys.size(); i++)
	{
		if (decoration_presence[keys[i]] == 1)
		{
			DecorationType dt = keys[i];
			if (decoration_support_probs.contains(dt))
			{
				QPair<DecorationType, QList<QPair<FurnitureType, double>>> pair;
				pair.first = dt;
				auto innerlist = Utility::QMap2QList(decoration_support_probs[dt]);
				qSort(innerlist.begin(), innerlist.end(), Utility::QPairSecondComparer());
				pair.second = innerlist;
				list.push_back(pair);
			}
		}
	}	
	return list;
}

QList<QPair<QString, QList<QPair<QString, double>>>> ProbLearning::GetDecorationTypesRandom(int n)
{
	QList<QPair<DecorationType, QList<QPair<FurnitureType, double>>>> list;
	n = n > sorted_decoration_types.size() ? sorted_decoration_types.size() : n;
	
	// copy list;
	for (size_t i = 0; i < sorted_decoration_types.size(); i++)
	{
		DecorationType dt = sorted_decoration_types[i].first;
		if (decoration_support_probs.contains(dt))
		{
			QPair<DecorationType, QList<QPair<FurnitureType, double>>> pair;
			pair.first = dt;
			auto innerlist = Utility::QMap2QList(decoration_support_probs[dt]);
			qSort(innerlist.begin(), innerlist.end(), Utility::QPairSecondComparer());			
			pair.second = innerlist;
			list.push_back(pair);
		}
	}

	// random remove
	int m = list.size() - n;
	for (size_t i = 0; i < m; i++)
	{
		list.removeAt(rand() % list.size());
	}
	return list;
}

double ProbLearning::GetFAll()
{
	return GetScoreAll(furniture_color_indices, decoration_presence);
}

void ProbLearning::LearnMI()
{
	m_islearned = false;
	m_useMI = true;
	furniture_color_clusters.clear();
	// 1. process files
	ReadInfoFromLabels();

	// 2. do statistics
	// 2.1 single furniture color	
	CalculateFunirtureColorMI();

	// 2.2 pairwise furniture colors
	//CalculateFurniturePairwiseColorProb();

	// 2.3 decoration mutual information
	//CulculateDecorationProb();

	// 2.4 decoration and furniture color corelation


	// 3. optimization
	//SimulatedAnnealing();
	srand(time(NULL));	

	QVector<FurnitureModel*> current_furniture_models = m_assets->GetFurnitureModels();
	int n = current_furniture_models.size();
	QVector<FurnitureType> types;
	for (size_t i = 0; i < n; i++)
	{
		types.push_back(current_furniture_models[i]->Type);
	}

	// aim: to get QMap<FurnitureType, ColorPalette*>

	// 0. initialize, send random clusterIndex to furniture
	furniture_color_indices.clear();
	for (size_t i = 0; i < n; i++)
	{
		auto map = furniture_color_probs[types[i]]; // 取出最大的cluster index
		auto keys = map.keys();
		QList<QPair<ClusterIndex, double>> sorted_clusters;
		for (size_t j = 0; j < keys.size(); j++)
		{
			sorted_clusters.push_back(QPair<ClusterIndex, double>(keys[j], map[keys[j]]));
		}
		qSort(sorted_clusters.begin(), sorted_clusters.end(), Utility::QPairSecondComparer());
		int index = sorted_clusters[0].first;	
		furniture_color_indices[types[i]] = index;
	}

	m_islearned = true;
	//QMap<FurnitureType,ColorPalette*> result = GetFurnitureColorPalette(1);
	auto list = GetDecorationTypesByNumber(15);
}


void ProbLearning::CalculateFunirtureColorMI()
{
	m_furniture_types = m_para->FurnitureTypes;
	QMap<FurnitureType, QVector<ColorPalette*>> furniture_color_palettes;
	// 取出所有样本中的颜色
	QVector<ImageFurnitureColorType> all_furniture_colors;
	int n_neg = m_furniture_colors[0].size();
	int n_pos = m_furniture_colors[1].size();
	for (size_t i = 0; i < n_neg; i++) // neg
	{
		all_furniture_colors.push_back(m_furniture_colors[0][i]);
	}
	for (size_t i = 0; i < n_pos; i++) // pos
	{
		all_furniture_colors.push_back(m_furniture_colors[1][i]);
	}

	for (size_t i = 0; i < all_furniture_colors.size(); i++)
	{
		ImageFurnitureColorType map = all_furniture_colors[i];
		for (size_t j = 0; j < map.keys().size(); j++)
		{
			if (!furniture_color_palettes.contains(map.keys()[j]))
			{
				QVector<ColorPalette*> palettes;
				palettes.push_back(map[map.keys()[j]]);
				furniture_color_palettes[map.keys()[j]] = palettes;
			}
			else
			{
				furniture_color_palettes[map.keys()[j]].push_back(map[map.keys()[j]]);
			}
		}
	}

	// 对每类家具聚类并统计概率
	for (size_t i = 0; i < m_furniture_types.size(); i++)
	{
		// 聚类
		QVector<ColorPalette*> colors = furniture_color_palettes[m_furniture_types[i]];
		vector<vector<int>> clusters = get_furniture_clusters(m_furniture_types[i], colors);
		// 统计
		QMap<ClusterIndex, double> map;
		for (size_t j = 0; j < clusters.size(); j++)
		{
			double N = colors.size(); // 总共的颜色数量
			double A = 0.0, B = 0.0;
			for (size_t k = 0; k < clusters[j].size(); k++) // 统计这个cluster中正样本的数量
			{
				if (colors[clusters[j][k]]->SampleType == Neg) // 判断是否是负样本				
					B++;				
				else
					A++;
			}
			double C = (double)n_pos - A;
			double MI = log((A*N + 0.01) / ((A + C)*(A + B) + 0.01));
			map[j] = 1 / (1 + exp(-MI));
		}
		furniture_color_probs[m_furniture_types[i]] = map;
	}
}


