#pragma once
#include <vector>
#include <Eigen/Dense>
using namespace Eigen;
using namespace std;

class MaximizeCoverageSelection
{

	/// <summary>
	/// Given a coverage matrix, select a set of columns that maximize the following 
	/// expression: sum_i max_j(coverageMatrix[i,j])
	/// The greedy algorithm acheives (1-1/e) approximation 
	/// Reference: An analysis of approximations for maximizing submodular set functions
	/// Reference: Diversity Maximization Under Matroid Constraints
	/// </summary>
	/// <param name="coverageMatrix">Each column represents a node for selection, each row represents a feature to cover</param>
	/// <param name="selectedNum">The number of representative nodes</param>
	/// <param name="initialSelections">the nodes that are representative nodes previously</param>
	/// <returns></returns>
private:
	static double *_deltaCov;
public:
	static vector<int> GetRepresentativeNodes(MatrixXd coverageMatrix,
		int selectedNum)
	{
		auto CMat = coverageMatrix;
		auto K = selectedNum;
		//auto iniS = initialSelections;		
		
		int ICnt = CMat.rows();
		int JCnt = CMat.cols();
		//int ICnt = CMat.GetLength(0);
		//int JCnt = CMat.GetLength(1);
		vector<int> S(K,0);	
		
		//int[] S = new int[K];
		_deltaCov = new double[K];

		double *maxCovByI = new double[ICnt];
		bool *bSelectedByJ = new bool[JCnt];			

		//start greedily pick
		for (int iter = 0; iter < K; iter++)
		{
			double maxDeltaCov = 0;
			int maxDeltaCovJ = -1;

			for (int j = 0; j < JCnt; j++)
			{
				if (bSelectedByJ[j])
					continue;
				double deltaCov = 0;
				for (int i = 0; i < ICnt; i++)
				{
					double delta = CMat[i, j] - maxCovByI[i];
					if (delta > 0)
						deltaCov += delta;
				}
				if (deltaCov > maxDeltaCov)
				{
					maxDeltaCov = deltaCov;
					maxDeltaCovJ = j;
				}
			}

			//set select
			int s = maxDeltaCovJ;
			for (int i = 0; i < ICnt; i++)
			{
				if (CMat[i, s] > maxCovByI[i])
					maxCovByI[i] = CMat[i, s];
			}
			S[iter] = s;
			_deltaCov[iter] = maxDeltaCov;
			bSelectedByJ[s] = true;
		}
		return S;
	}
	static vector<int> GetRepresentativeNodes(MatrixXd coverageMatrix,
		int selectedNum, double *deltaCov)
	{
		auto repNodes = GetRepresentativeNodes(coverageMatrix, selectedNum);
		deltaCov = _deltaCov;
		return repNodes;
	}
};
