#include "ClusterMethods.h"

#include <map>
#include <math.h>
#include "SpectralCluster.h"
//#include "NMI.h"
using namespace std;

vector<vector<int>> ClusterMethods::getHierarchicalClusters(int flag)
{
	// initial cluster: every model forms a cluster
	hierarchical_cluster.clear();
	for (int i = 0; i < m_sample_num; i++)
	{
		vector<int> cluster;
		cluster.push_back(i);
		hierarchical_cluster.push_back(cluster);
	}

	// terminate condition: cluster number >= 150
	while (hierarchical_cluster.size() > m_cluster_num)
	{
		// find the minimum cluster-cluster distance or similarity
		double min_distance = INT_MAX;
		int nearest_c1 = 0, nearest_c2 = 1;
		for (int i = 0; i < hierarchical_cluster.size(); i++)
		{
			vector<int> t_cluster = hierarchical_cluster[i];
			for (int j = i + 1; j < hierarchical_cluster.size(); j++)
			{
				double tmp_dis = get_cluster_similarity(flag, hierarchical_cluster[i], hierarchical_cluster[j]);
				if (min_distance > tmp_dis)
				{
					min_distance = tmp_dis;
					nearest_c1 = i;
					nearest_c2 = j;
				}
			}
		}
		// merge nearest c1 and c2
		merge_clusters(nearest_c1, nearest_c2);
	}

	return hierarchical_cluster;
}


double ClusterMethods::get_cluster_similarity(int flag, vector<int> &c1, vector<int> &c2)
{
	double cluster_distance = 0.0;
	double distance_min = INT_MAX, distance_max = 0.0;
	double distance_avg = 0.0;
	int n1 = c1.size();
	int n2 = c2.size();

	for (int i = 0; i < n1; i++)
	{
		for (int j = 0; j < n2; j++)
		{
			double tmp_sim = distance_matrix[c1[i]][c2[j]];
			if (distance_min > tmp_sim)
			{
				distance_min = tmp_sim;
			}
			if (distance_max < tmp_sim)
			{
				distance_max = tmp_sim;
			}

			distance_avg += tmp_sim / ((double)n1*n2);
		}
	}

	switch (flag)
	{
	case HC_MIN_DISTANCE:
		cluster_distance = distance_min;
		break;
	case HC_MAX_DISTANCE:
		cluster_distance = distance_max;
		break;
	case HC_AVG_DISTANCE:
		cluster_distance = distance_avg;
		break;
	default:
		break;
	}

	return cluster_distance;
}


void ClusterMethods::merge_clusters(int c1_index, int c2_index)
{
	vector<int> c1 = hierarchical_cluster[c1_index];
	vector<int> c2 = hierarchical_cluster[c2_index];
	// 	if (c1.size() > c2.size())	// merge c2 to c1
	// 	{
	for (int i = 0; i < c2.size(); i++)
	{
		hierarchical_cluster[c1_index].push_back(c2[i]);
	}
	// erase c2
	vector<vector<int>>::iterator iter = hierarchical_cluster.begin() + c2_index;
	hierarchical_cluster.erase(iter);
}



inline int getModelID(int modelID)
{
	return modelID > 762 ? modelID - 1 : modelID;
}


vector<int> ClusterMethods::getBenchmark(string filename)
{

	ifstream in_cla(filename);
	string buf;
	
	getline(in_cla, buf); // PSB 1
	getline(in_cla, buf);
	sscanf(buf.c_str(), "%d %d", &m_cluster_num, &m_sample_num);
	m_cluster_num = 0;
	
	vector<int> cluster_bm(m_sample_num, -1);


	char bufferline[256];
	char buf1[50], buf2[50];
	int label_ID = -1;
	while (!in_cla.eof())
	{
		in_cla.getline(bufferline, 100);
		string strline = bufferline;

		if (strline.length() > 0)
		{
			//char *buffer = buf.data();
			string c_name, c_parent;
			int modlenum = 0;
			sscanf(bufferline, "%s %s %d", buf1, buf2, &modlenum);
			if (modlenum > 0)
			{
				m_cluster_num++; // clusters with at least one element
				label_ID++;
			}
			for (int i = 0; i < modlenum; i++)
			{
				int modelID;
				in_cla.getline(bufferline, 50);
				sscanf(bufferline, "%d", &modelID);
				cluster_bm[getModelID(modelID)] = label_ID;
			}
		}
	}
	return cluster_bm;
}

vector<vector<int>> ClusterMethods::getKMeansClusters()
{
	// initial clusters	
	kmeans_cluster.clear();
	for (int i = 0; i < m_cluster_num; i++)
	{
		vector<int> cluster;
		kmeans_cluster.push_back(cluster);
	}
	//srand((unsigned)time(NULL)); 

	// distribute samples randomly to all clusters
	for (int i = 0; i < m_sample_num; i++)
	{
		kmeans_cluster[rand() % m_cluster_num].push_back(i);
	}

	int iterate_times = 0;
	vector<vector<int>> new_cluster(m_cluster_num);
	while (iterate_times++ < K_MEANS_ITERATE_TIMES) // 
	{
		for (int i = 0; i < m_cluster_num; i++)
		{
			new_cluster[i].clear();
		}

		for (int i = 0; i < m_sample_num; i++)
		{
			double min_distance = INT_MAX;
			int min_cluster_label = 0;
			int src_clusterID, src_cluster_pos;
			for (int j = 0; j < m_cluster_num; j++)
			{

				// use average sim
				double sum_distance = 0.0;
				int n = kmeans_cluster[j].size();
				for (int k = 0; k < kmeans_cluster[j].size(); k++)
				{
					// it's the cluster model i currently belongs to
					if (i == kmeans_cluster[j][k])
					{
						n--; // dont contain i to i 
					}
					else
					{
						sum_distance += distance_matrix[i][kmeans_cluster[j][k]];
					}
				}
				sum_distance /= n;
				if (min_distance > sum_distance)
				{
					min_distance = sum_distance;
					min_cluster_label = j;
				}

				// 				//use nearest sim
				// 				int min_sim = INT_MAX;
				// 				for (int k = 0;k < kmeans_cluster[j].size();k++)
				// 				{			
				// 					if (sim_matrix[i][kmeans_cluster[j][k]] == 0)
				// 					{
				// 						src_clusterID = j;
				// 						src_cluster_pos = k;
				// 						continue;
				// 					}
				// 					if (min_sim > sim_matrix[i][kmeans_cluster[j][k]])
				// 					{
				// 						min_sim = sim_matrix[i][kmeans_cluster[j][k]];
				// 					}					
				// 				}				
				// 				if (min_distance > min_sim)
				// 				{
				// 					min_distance = min_sim;
				// 					min_cluster_label = j;
				// 				}		


			}
			// move i from src cluster to new cluster
			//  			kmeans_cluster[src_clusterID].erase(kmeans_cluster[src_clusterID].begin() + src_cluster_pos);
			//  			kmeans_cluster[min_cluster_label].push_back(i);
			new_cluster[min_cluster_label].push_back(i);

		}
		kmeans_cluster = new_cluster;
	}

	return kmeans_cluster;
}

vector<vector<int>> ClusterMethods::getKMeansClustersAvgSim()
{
	vector<int> random_center;			// k-means center
	vector<vector<int>> model_tag;	// tag-index, list of modelID

	int iterate_times = 500;

	for (int i = 0; i < m_cluster_num; i++)
	{
		random_center.push_back(rand() % m_sample_num);
		vector<int> tags;
		model_tag.push_back(tags);
	}

	// k-means ierate times
	for (int k = 0; k < iterate_times; k++)
	{
		// clean every tag groups
		for (int i = 0; i < m_cluster_num; i++)
		{
			model_tag[i].clear();
		}
		// find the nearest cluster for each model
		for (int i = 0; i < m_sample_num; i++)
		{
			double mindistance = distance_matrix[i][random_center[0]];
			int tag = 0;
			for (int j = 1; j < m_cluster_num; j++)
			{
				if (mindistance > distance_matrix[i][random_center[j]])
				{
					mindistance = distance_matrix[i][random_center[j]];
					tag = j;
				}
			}
			model_tag[tag].push_back(i);
		}

		// update cluster center
		for (int i = 0; i < m_cluster_num; i++)
		{
			int min_avg_sim = INT_MAX;
			//int new_cluster_center = random_center[i];

			for (int m = 0; m< model_tag[i].size(); m++)
			{
				int sim_sum = 0;
				for (int n = 0; n < model_tag[i].size(); n++)
				{
					sim_sum += distance_matrix[m][n];
				}
				sim_sum /= model_tag[i].size();
				if (min_avg_sim > sim_sum)
				{
					min_avg_sim = sim_sum;
					random_center[i] = model_tag[i][m];
				}
			}
		}
	}

	int pas = 0;

	return model_tag;
}

vector<vector<int>> ClusterMethods::getKMedoidsClusters(int iterate_times, vector<int> &random_center)
{

	vector<vector<int>> model_tag(m_cluster_num);	// tag-index, list of modelID

												// 	for (int i = 0;i < m_cluster_num;i++ )
												// 	{
												// 		vector<int> tags;		
												// 		model_tag.push_back(tags);		
												// 	} 	 

	// init
	// find the nearest cluster for each model
	for (int i = 0; i < m_sample_num; i++)
	{
		double mindistance = distance_matrix[i][random_center[0]];
		int tag = 0;
		for (int j = 1; j < m_cluster_num; j++)
		{
			if (mindistance > distance_matrix[i][random_center[j]])
			{
				mindistance = distance_matrix[i][random_center[j]];
				tag = j;
			}
		}
		model_tag[tag].push_back(i);
	}
												// k-medoids iterate times
	for (int k = 0; k < iterate_times; k++)
	{
		// update cluster center
		bool flag = false; // if converged
		for (int i = 0; i < m_cluster_num; i++)
		{
			double min_avg_sim = INT_MAX;
			//int new_cluster_center = random_center[i];

			for (int m = 0; m< model_tag[i].size(); m++)
			{
				double sim_sum = 0;
				for (int n = 0; n < model_tag[i].size(); n++)
				{
					sim_sum += distance_matrix[m][n];
				}
				sim_sum /= model_tag[i].size();
				if (min_avg_sim > sim_sum)
				{
					min_avg_sim = sim_sum;
					if (random_center[i] != model_tag[i][m])
					{
						flag = true; // ±‰ªØ¡À
					}
					random_center[i] = model_tag[i][m];
				}
			}
		}
		if (!flag)
		{
			cout << "k-medoids converged at " << k << endl;
			break;
		}



		// clean every tag groups
		for (int i = 0; i < m_cluster_num; i++)
		{
			model_tag[i].clear();
		}
		// find the nearest cluster for each model
		for (int i = 0; i < m_sample_num; i++)
		{
			double mindistance = distance_matrix[i][random_center[0]];
			int tag = 0;
			for (int j = 1; j < m_cluster_num; j++)
			{
				if (mindistance > distance_matrix[i][random_center[j]])
				{
					mindistance = distance_matrix[i][random_center[j]];
					tag = j;
				}
			}
			model_tag[tag].push_back(i);
		}

		
	}

	int pas = 0;

	return model_tag;

}

vector<vector<int>> ClusterMethods::getKMedoidsClusters(int iterate_times)
{
	vector<int> random_center;			// k-means center
	vector<vector<int>> model_tag;	// tag-index, list of modelID


	vector<int> flag(m_sample_num, 0);
	for (int i = 0; i < m_cluster_num; i++)
	{
		while (1)
		{
			int id = rand() % m_sample_num;
			if (flag[id] == 0)
			{
				random_center.push_back(id);
				flag[id] = 1; // already added
				break;
			}
		}

		vector<int> tags;
		model_tag.push_back(tags);
	}

	// k-medoids ierate times
	for (int k = 0; k < iterate_times; k++)
	{
		// clean every tag groups
		for (int i = 0; i < m_cluster_num; i++)
		{
			model_tag[i].clear();
		}
		// find the nearest cluster for each model
		for (int i = 0; i < m_sample_num; i++)
		{
			int mindistance = distance_matrix[i][random_center[0]];
			int tag = 0;
			for (int j = 1; j < m_cluster_num; j++)
			{
				if (mindistance > distance_matrix[i][random_center[j]])
				{
					mindistance = distance_matrix[i][random_center[j]];
					tag = j;
				}
			}
			model_tag[tag].push_back(i);
		}

		// update cluster center
		for (int i = 0; i < m_cluster_num; i++)
		{
			int min_avg_sim = INT_MAX;
			//int new_cluster_center = random_center[i];

			for (int m = 0; m< model_tag[i].size(); m++)
			{
				int sim_sum = 0;
				for (int n = 0; n < model_tag[i].size(); n++)
				{
					sim_sum += distance_matrix[m][n];
				}
				sim_sum /= model_tag[i].size();
				if (min_avg_sim > sim_sum)
				{
					min_avg_sim = sim_sum;
					random_center[i] = model_tag[i][m];
				}
			}
		}

	}

	int pas = 0;

	return model_tag;
}



vector<int> ClusterMethods::getRandomCluster()
{
	vector<int> random_clusters(m_sample_num);
	for (int i = 0; i < m_sample_num; i++)
	{
		random_clusters[i] = rand() % m_cluster_num;
	}
	return random_clusters;
}


vector<vector<int>> ClusterMethods::getSpectralClusters(int iterate_times, int dimension_num, int t)
{
	//int K = 53;
	SpectralCluster spectral_cluster(distance_matrix, dimension_num,m_cluster_num,t);
	vector<vector<int>> clusters = spectral_cluster.getSpectralClusters(m_cluster_num, iterate_times, t);

	return clusters;
}

//double ClusterMethods::GetNMI(vector<int> &label1, vector<int> &label2)
//{
//	NMI nmi_calculator;
//	return nmi_calculator.GetNormalizedMutualInfo(label1, label2);
//}






