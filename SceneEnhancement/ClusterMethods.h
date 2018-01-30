#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;


//#define MODEL_NUM 1814
//#define CLUSTER_NUM 53
#define K_MEANS_ITERATE_TIMES 100

#define HC_MAX_DISTANCE 0
#define HC_MIN_DISTANCE 1
#define HC_AVG_DISTANCE 2

class ClusterMethods
{
public:
	ClusterMethods(): m_sample_num(0), m_cluster_num(0)
	{}
	ClusterMethods(vector<vector<double>> dismatrix, int clusternum)
	{
		distance_matrix = dismatrix;
		m_cluster_num = clusternum;
		m_sample_num = dismatrix.size();
	}

	// return a list of clusters	
	vector<vector<int>> getHierarchicalClusters(int flag = 0);
	vector<vector<int>> getSpectralClusters(int iterate_times = 100, int dimension_num = 50, int t = 1);
	

	// 以下还没有经过验证
	vector<vector<int>> getKMeansClusters();
	vector<vector<int>> getKMeansClustersAvgSim();
	vector<vector<int>> getKMedoidsClusters(int iterate_times, vector<int> &random_center);
	vector<vector<int>> getKMedoidsClusters(int iterate_times);
	

	// calculate nmi
	//double GetNMI(vector<int> &label1, vector<int> &label2);
	vector<int> getBenchmark(string filename);
	vector<int> getRandomCluster();
	//vector<int> getMeanShiftClusters();

	// data
	int getClusterNum() { return m_cluster_num; }

private:
	//
	int m_sample_num;
	int m_cluster_num;
	//int m_model_num;
	// data
	vector<vector<double>> distance_matrix;

	// nmi

	// data structure 0 -hierarchical cluster
	// vector of clusters, each cluster consisits its model ID, 762 is different
	vector<vector<int>> hierarchical_cluster;
	vector<vector<int>> kmeans_cluster;
	//vector<vector<int>> meanshift_cluster;

	// hierarchical
	void merge_clusters(int c1_index, int c2_index);
	double get_cluster_similarity(int flag, vector<int> &c1, vector<int> &c2);

};


