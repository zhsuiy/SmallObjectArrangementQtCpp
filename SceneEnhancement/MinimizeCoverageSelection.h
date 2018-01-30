#pragma once
#include <vector>
#include <Eigen/Dense>
using namespace Eigen;
using namespace std;

class MinimizeCoverageSelection
{
public:
	static vector<int> GetRepresentativeNodes(double** coverageMatrix, int rows, int cols,
		int selectedNum)
	{
		auto CMat = coverageMatrix;
		int K = selectedNum;

		int ICnt = rows;
		int JCnt = cols;
		vector<int> S(K, 0);
		double * _deltaCov = new double[K];

		double *minCovByI = new double[ICnt];
		bool *bSelectedByJ = new bool[JCnt];

		for (int i = 0; i < ICnt; i++)
		{
			minCovByI[i] = 100;
		}
		for (size_t i = 0; i < JCnt; i++)
		{
			bSelectedByJ[i] = false;
		}

		//start greedily pick
		for (int iter = 0; iter < K; iter++)
		{
			double maxDeltaCov = -1;
			int minDeltaCovJ = -1;

			for (int j = 0; j < JCnt; j++)
			{
				if (bSelectedByJ[j])
					continue;
				double deltaCov = 0;
				for (int i = 0; i < ICnt; i++)
				{
					double delta = minCovByI[i] - CMat[i][j];
					if (delta > 0)
						deltaCov += delta;
				}
				if (deltaCov > maxDeltaCov)
				{
					maxDeltaCov = deltaCov;
					minDeltaCovJ = j;
				}
			}

			//set select
			int s = minDeltaCovJ;
			for (int i = 0; i < ICnt; i++)
			{
				if (CMat[i][s] < minCovByI[i])
					minCovByI[i] = CMat[i][s];
			}
			S[iter] = s;
			_deltaCov[iter] = maxDeltaCov;
			bSelectedByJ[s] = true;
		}
		return S;
	}
};
