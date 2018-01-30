#pragma once
#pragma once
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

class KMeans
{
public:
	KMeans(vector<vector<double>> &coords, int clusternum, int max_iterator_times);
	~KMeans();

	vector<vector<int>> getKMeansClusters();

private:
	vector<vector<double>> m_coordinates;
	int m_cluster_num;
	int m_sample_num;
	int m_iterator_times;


};
