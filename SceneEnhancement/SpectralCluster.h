#pragma once
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>
#include <algorithm>
#include <complex>
#include "KMeans.h"
using namespace std;

// #pragma push_macro("complex")
// #undef complex

#include <Eigen/Sparse>
#include <Eigen/Eigenvalues>

using namespace Eigen;

/*#pragma pop_macro("complex")*/
class SpectralCluster
{
public:
	SpectralCluster(vector<vector<double>> &disM, int reduce_dimension, int clusternum, int temporature);
	~SpectralCluster();

	vector<vector<int>> getSpectralClusters(int K, int max_iterate_times, int t = 1); // K is the K-means 
	void testEigen();
private:
	vector<vector<double>> distance_matrix;
	vector<vector<double>> euclidean_coords;
	int m_reduced_dimension;
	int m_cluster_num;
	int m_temperature;
	int m_samle_num;
	bool m_init;
private:
	// spectral cluster
	vector<vector<double>> getSparseMatrix(int K, bool isSim);
	vector<vector<double>> getDiagnalSumMatrix(vector<vector<double>> &spm);
	vector<vector<double>> getNormalizedSimMatrix(vector<vector<double>> &spM, vector<vector<double>> &diagM);
	vector<vector<double>> getLapLacianMatrix();
	vector<vector<double>> getEuclideanCoords(int t = 1);
	//vector<vector<int>> getKMeansCluster(vector<vector<double>> &coords, int K);



};

