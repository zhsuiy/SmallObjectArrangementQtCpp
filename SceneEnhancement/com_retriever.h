#pragma once
#include "comb_perm.h"
#include <vector>
#include <numeric>
using namespace std;
class CombinationRetriever
{
	unsigned len;
	std::uint64_t count;

public:
	vector<vector<int>> all_combs;
	explicit CombinationRetriever(unsigned l) : len(l), count(0) {}

	template <class It>
	bool operator()(It first, It last)  // called for each permutation
	{
		// count the number of times this is called
		++count;
		// print out [first, mid) surrounded with [ ... ]
		vector<int> cur_comb;
		unsigned r = 0;
		cur_comb.push_back(*first);
		++r;
		for (++first; first != last; ++first)
		{
			cur_comb.push_back(*first);
			++r;
		}

		all_combs.push_back(cur_comb);

		return false;
	}

	operator std::uint64_t() const { return count; }
};

vector<vector<int>> GetCombination(int n, int k, vector<int> v)
{	
	std::vector<int>::iterator r = v.begin() + k;
	CombinationRetriever f1(v.size());
	auto handle = for_each_combination(v.begin(), r, v.end(), f1);
	return handle.all_combs;
}

void getCombinationRecursive(vector<vector<int>> &ret, vector<int> prev, vector<int> cur, int m, int n)
{
	if (prev.size() == n)
	{
		ret.push_back(prev);
		return;
	}
	vector<vector<int>> comb;
	if (cur.size() <= m)
		comb.push_back(cur);
	else
		comb = GetCombination(cur.size(), m, cur);
	for (auto it : comb)
	{
		auto tmp0 = prev;
		auto tmp1 = cur;
		prev.insert(prev.end(), it.begin(), it.end());
		vector<int> newvec;
		for (auto c : cur)
		{
			if (find(it.begin(), it.end(), c) == it.end())
				newvec.push_back(c);
		}
		cur = newvec;
		getCombinationRecursive(ret, prev, cur, m, n);
		prev = tmp0;
		cur = tmp1;
	}
}

vector<vector<vector<int>>> getIndexPerLayer(int n_objects, int m_layers)
{
	vector<vector<vector<int>>> all_combs_layers;

	std:vector<int> v(n_objects);
	std::iota(v.begin(), v.end(), 0);
	vector<vector<int>> ret;
	vector<int> prev;
	int obj_per_layer = std::ceil((double)n_objects / m_layers);
	getCombinationRecursive(ret, prev, v, obj_per_layer, n_objects);

	for (auto c : ret)
	{
		vector<vector<int>> index_per_layer;
		vector<int>::iterator it = c.begin();
		for (size_t i = 0; i < m_layers - 1; i++)
		{
			vector<int> indices;
			indices.insert(indices.end(), it, it + obj_per_layer);
			it += obj_per_layer;
			index_per_layer.push_back(indices);
		}
		// last layer
		vector<int> indices;
		indices.insert(indices.end(), c.begin() + obj_per_layer*(m_layers - 1), c.end());
		index_per_layer.push_back(indices);

		all_combs_layers.push_back(index_per_layer);
	}

	return all_combs_layers;
}