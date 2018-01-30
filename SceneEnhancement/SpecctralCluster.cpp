#include "SpectralCluster.h"


SpectralCluster::SpectralCluster(vector<vector<double>> &disM, int reduce_dimension, int clusternum, int temperature)
{
	m_init = false;
	distance_matrix = disM;
	m_reduced_dimension = reduce_dimension;
	m_cluster_num = clusternum;
	m_temperature = temperature;
	m_samle_num = disM.size();
	m_init = true;
}

SpectralCluster::~SpectralCluster()
{
}

struct CmpByValue
{
	bool operator() (const pair<int, double>& lhs, const pair<int, double>& rhs)
	{
		return lhs.second < rhs.second;
	}
};


struct CmpByRealValue
{
	bool operator() (const pair<int, complex<double>>& lhs, const  pair<int, complex<double>>& rhs)
	{
		return lhs.second.real() > rhs.second.real();
	}
};

vector<vector<double>> SpectralCluster::getSparseMatrix(int K, bool isSim)
{
	typedef int INDEX;
	typedef double DISTANCE;
	vector<vector<double>> sparse_distance_matrix(m_samle_num, vector<double>(m_samle_num, 0.0));
	double theta1 = 4000;

	for (int i = 0; i < distance_matrix.size(); i++)
	{
		double min_distance = INT_MAX;
		vector<pair<INDEX, DISTANCE>> disvec;
		for (int j = 0; j < distance_matrix[i].size(); j++)
		{
			disvec.push_back(pair<INDEX, DISTANCE>(j, distance_matrix[i][j]));
		}
		sort(disvec.begin(), disvec.end(), CmpByValue());
		for (int j = 0; j < K; j++) // KNN K=250
		{
			double dis = distance_matrix[i][disvec[j].first];
			// so it's now sim_matrix, 0 for different,1 for similar
			sparse_distance_matrix[i][disvec[j].first] = isSim ? exp(-dis*dis / (theta1*theta1)) : dis;
			sparse_distance_matrix[disvec[j].first][i] = isSim ? exp(-dis*dis / (theta1*theta1)) : dis;
			/*sparse_distance_matrix[i][disvec[j].first] = exp(-dis*dis/(theta1*theta1))*/;
			// 
		}
	}

	return sparse_distance_matrix;
}

vector<vector<double>> SpectralCluster::getDiagnalSumMatrix(vector<vector<double>> &spm)
{
	vector<vector<double>> diagnalM(m_samle_num, vector<double>(m_samle_num, 0.0));
	for (int i = 0; i < spm.size(); i++)
	{
		double sum = 0.0;
		for (int j = 0; j < spm[i].size(); j++)
		{
			sum += spm[i][j];
		}
		diagnalM[i][i] = sum;
	}
	return diagnalM;
}

vector<vector<double>> SpectralCluster::getNormalizedSimMatrix(vector<vector<double>> &spM, vector<vector<double>> &diagM)
{
	vector<vector<double>> normalizedSimM(m_samle_num, vector<double>(m_samle_num, 0.0));
	for (int i = 0; i < spM.size(); i++)
	{
		for (int j = 0; j < spM[i].size(); j++)
		{
			normalizedSimM[i][j] = spM[i][j] / diagM[i][i];
		}
	}
	return normalizedSimM;
}

vector<vector<double>> SpectralCluster::getLapLacianMatrix()
{
	vector<vector<double>> sparse_dis_m = getSparseMatrix(6, false);
	vector<vector<double>> laplacianM(m_samle_num, vector<double>(m_samle_num, 0.0));
	for (int i = 0; i < sparse_dis_m.size(); i++)
	{
		double sum = 0.0;
		for (int j = 0; j < sparse_dis_m[i].size(); j++)
		{
			sum += sparse_dis_m[i][j];
		}
		for (int j = 0; j < sparse_dis_m[i].size(); j++)
		{
			if (i == j)
			{
				laplacianM[i][j] = sum - sparse_dis_m[i][j];
			}
			else
			{
				laplacianM[i][j] = -sparse_dis_m[i][j];
			}

		}
	}

	return laplacianM;
}

vector<vector<int>> SpectralCluster::getSpectralClusters(int K, int max_iterate_times, int t)
{
	vector<vector<int>> clusters;
	// make sure that the distance matrix is not empty
	if (distance_matrix.size() > 0) 
	{
		euclidean_coords = getEuclideanCoords(t);
		KMeans kmeans_cluster(euclidean_coords, K, max_iterate_times);
		clusters = kmeans_cluster.getKMeansClusters();
	}	
	return clusters;
}

// t is the diffusion times
vector<vector<double>> SpectralCluster::getEuclideanCoords(int t)
{
	cout << "this is a test" << endl;

	ofstream out("matrixtime.txt");

	// Laplacian
	// 	vector<vector<double>> laplacianM = getLapLacianMatrix();
	// 	MatrixXd m(MODEL_NUM,MODEL_NUM); // similarity matrix
	// 	for (int i = 0;i < laplacianM.size();i++)
	// 	{
	// 		for (int j = 0;j < laplacianM[i].size();j++)
	// 		{
	// 			m(i,j) = laplacianM[i][j];
	// 		}
	// 	}

	// laplacian similarity
	// 	vector<vector<double>> simM = getSparseMatrix(6,true);
	// 	vector<vector<double>> diagM = getDiagnalSumMatrix(simM);
	// 	vector<vector<double>> LaplM = substract(diagM,simM);
	// 	vector<vector<double>> diagL = getDiagnalSumMatrix(LaplM);
	// 	vector<vector<double>> normL = getNormalizedSimMatrix(LaplM,diagL); 	
	// 	MatrixXd m(MODEL_NUM,MODEL_NUM); // similarity matrix
	// 	for (int i = 0;i < normL.size();i++)
	// 	{
	// 		for (int j = 0;j < normL[i].size();j++)
	// 		{
	// 			m(i,j) = normL[i][j];
	// 		}
	// 	}

	MatrixXd a;
	MatrixXd b;


	// Hu
	MatrixXd W(m_samle_num, m_samle_num), D(m_samle_num, m_samle_num);
	vector<vector<double>> simM = getSparseMatrix(m_samle_num < 10 ? m_samle_num : 10, true);
	vector<vector<double>> diagM = getDiagnalSumMatrix(simM);
	for (int i = 0; i < simM.size(); i++)
	{
		for (int j = 0; j < simM[i].size(); j++)
		{
			W(i, j) = simM[i][j];
			D(i, j) = diagM[i][j];
		}
	}
	//SelfAdjointEigenSolver<MatrixXd> esd(D);
	MatrixXd Z = D.inverse()*W;
	// 	MatrixXd DiffusionZ = MatrixXd::Identity(MODEL_NUM,MODEL_NUM);
	// 	for (int i = 0;i < t;i++)
	// 	{
	// 		DiffusionZ = DiffusionZ*Z;
	// 	}

	//Z=Z.
	//  	vector<vector<double>> normalizedM = getNormalizedSimMatrix(simM,diagM); 	
	// 	MatrixXd m(MODEL_NUM,MODEL_NUM); // similarity matrix
	// 	for (int i = 0;i < normalizedM.size();i++)
	// 	{
	// 		for (int j = 0;j < normalizedM[i].size();j++)
	// 		{
	// 			m(i,j) = normalizedM[i][j];
	// 		}
	// 	}	

	clock_t start, finish;
	start = clock();

	//MatrixXd m = MatrixXd::Random(MODEL_NUM,MODEL_NUM);
	EigenSolver<MatrixXd> es;
	es.compute(Z, true);

	finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "duration time: " << duration << endl;
	out << "duration time: " << duration << endl;


	vector<pair<int, complex<double>>> eigenVal;
	int valueNum = es.eigenvalues().size();

	for (int i = 0; i < valueNum; i++)
	{
		eigenVal.push_back(pair<int, complex<double>>(i, es.eigenvalues()[i]));
	}

	out << endl << endl;
	out << "Largest eigenvaluse:" << endl;
	sort(eigenVal.begin(), eigenVal.end(), CmpByRealValue());
	for (int i = 0; i < valueNum; i++)
	{
		out << i << ":" << eigenVal[i].second.real() << " , " << eigenVal[i].second.imag() << endl;
	}

	vector<VectorXcd> eigenVectors;
	int vecNum = es.eigenvectors().size() / m_samle_num;
	for (int i = 0; i < vecNum; i++)
	{
		eigenVectors.push_back(es.eigenvectors().col(i));
	}
	MatrixXcd eigenV = es.eigenvectors();

	// 	vector<double> vectors;
	// 	vectors.push_back(eigenV(0,0).real());


	out << endl << endl;
	out << "eigenvectors: " << endl;
	for (int i = 0; i < m_samle_num; i++)
	{
		out << "vector " << i << ":" << endl;
		out << eigenVectors[eigenVal[0].first].row(i).real() << "," << eigenVectors[0].row(i).imag() << endl;
	}


	vector<vector<double>> new_coords(m_samle_num, vector<double>(m_reduced_dimension, 0.0));
	for (int i = 0; i < m_reduced_dimension; i++)
	{
		//VectorXcd eigenvec = eigenVectors[eigenVal[i].first];
		for (int j = 0; j < m_samle_num; j++)
		{
			new_coords[j][i] = pow(eigenVal[i].second.real(), t - 1)*eigenV(j, eigenVal[i].first).real();
		}
	}

	// for test
	// 	out << "this is for test..." << endl;
	// 	for (int i = 0;i < MODEL_NUM;i++)
	// 	{
	// 		out << "No. ";
	// 		for (int j = 0;j < m_reduced_dimension;j++)
	// 		{
	// 			out <<  i << "\t" << new_coords[i][j]; 
	// 		}
	// 		cout << endl;
	// 	}

	out.close();

	return new_coords;
}

void SpectralCluster::testEigen()
{
	fstream out("Eigentest.txt");

	MatrixXd m(3, 3);

	m(0, 0) = 1;		m(0, 1) = 0;		m(0, 2) = 1;
	m(1, 0) = 0;		m(1, 1) = 2;		m(1, 2) = 1;
	m(2, 0) = 0;		m(2, 1) = 0;		m(2, 2) = 3;

	EigenSolver<MatrixXd> es(m);
	MatrixXcd eigenV = es.eigenvectors();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			out << eigenV(i, j) << "\t";
		}
		out << endl;
	}
	out.close();


}