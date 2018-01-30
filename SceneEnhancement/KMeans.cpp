#include "KMeans.h"

KMeans::KMeans(vector<vector<double>> &coords, int K, int iterator_times)
{
	m_coordinates = coords;
	m_sample_num = coords.size();
	m_cluster_num = K;
	m_iterator_times = iterator_times;
}

KMeans::~KMeans()
{
}

inline double getDistance(vector<double> p1, vector<double> p2)
{
	double sum = 0.0;
	for (int i = 0; i < p1.size(); i++)
	{
		sum += (p1[i] - p2[i])*(p1[i] - p2[i]);
	}
	return sum;
}

vector<vector<int>> KMeans::getKMeansClusters()
{
	// init
	vector<vector<int>> model_tag;
	int count = 0;
	vector<vector<double>> centers;
	vector<int> flag(m_sample_num, 0);
	for (int i = 0; i < m_cluster_num; i++)
	{
		while (1)
		{
			int id = rand() % m_cluster_num;
			if (flag[id] == 0)
			{
				centers.push_back(m_coordinates[id]);
				flag[id] = 1; // already added
				break;
			}
		}
		vector<int> tags;
		model_tag.push_back(tags);
	}

	// cluster	
	while (count < m_iterator_times)
	{
		for (int i = 0; i < model_tag.size(); i++)
		{
			model_tag[i].clear();
		}

		// cluster
		for (int i = 0; i < m_coordinates.size(); i++)
		{
			double distance = INT_MAX;
			int cluster_tag = 0;
			for (int j = 0; j < centers.size(); j++)
			{
				if (distance > getDistance(m_coordinates[i], centers[j]))
				{
					cluster_tag = j;
					distance = getDistance(m_coordinates[i], centers[j]);
				}
			}
			model_tag[cluster_tag].push_back(i);

		}


		// update centers		
		bool flag = true;
		for (int i = 0; i < centers.size(); i++) // for every center
		{

			vector<int> cluster = model_tag[i]; // find its corresponding cluster
			for (int j = 0; j < centers[i].size(); j++) // for every dimension
			{
				double coord = 0.0;
				for (int k = 0; k < cluster.size(); k++) // sum up all the coodinates of this cluster
				{
					coord += m_coordinates[cluster[k]][j];
				}
				double tmp = coord / cluster.size(); // calculate average
				if (centers[i][j] != tmp)
				{
					centers[i][j] = tmp;
					flag = false;
				}
			}
		}
		if (flag) // unchanged
		{
			break;
		}

		count++;
	}

	ofstream out("log.txt", iostream::app);
	out << "Kmeans iteration time: " << count << endl;
	out.close();

	return model_tag;

}
