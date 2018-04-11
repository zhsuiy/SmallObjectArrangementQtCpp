#pragma once
#include <iostream>
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
#include <QtCore/qvector.h>
#include "Parameter.h"

using namespace std;
typedef QString CatName;
enum Relation {Greater, Less, Equal};

class SmallObjectArrange
{
public:
	SmallObjectArrange();
	~SmallObjectArrange();

	void InitArranger();
	void UpdateUserPreferences(QVector<QPair<QPair<CatName, CatName>, Relation>> height_pref,
		QVector<QPair<QPair<CatName, CatName>, Relation>> medium_pref,
		QVector<QPair<QPair<CatName, CatName>, Relation>> depth_pref);
	void PropagateUserPreference();
	
	QMap<CatName, int> GetCatIndexMapping() {
		return map_cat_index;
	};
	QVector<QVector<float>> GetHeightEqualProb() {
		return cat_pair_equal_height;
	}
	QVector<QVector<float>> GetMediumEqualProb() {
		return cat_pair_equal_medium;
	}
	QVector<QVector<float>> GetDepthEqualProb() {
		return cat_pair_equal_depth;
	}
	QVector<QVector<float>> GetHeightHigherProb() {
		return cat_pair_height;
	}
	QVector<QVector<float>> GetMidiumMiddleProb() {
		return cat_pair_medium;
	}
	QVector<QVector<float>> GetDepthFrontProb() {
		return cat_pair_depth;
	}
	
private:
// hyper parameters
	QString uid = "r30-d10";
	float m_pair_pair_sim_threshold = 0;
	int m_neighbor_num = 10;
	// how many rounds to get extend variables when propagating uneuqal prefrences
	int m_variable_round_unequal = 2;
	int m_variable_round_equal = 2;
	float m_lambda_equal = 10;
	float m_lambda_unequal = 10;
// members
	Parameter *m_para;	
	QVector<CatName> all_cats;
	QMap<CatName, int> map_cat_index;
	QMap<CatName, float> map_cat_prob;

	QVector<QVector<float>> cat_pair_height;
	QVector<QVector<float>> cat_pair_medium;
	QVector<QVector<float>> cat_pair_depth;
	
	QVector<QVector<float>> cat_pair_equal_height;
	QVector<QVector<float>> cat_pair_equal_medium;
	QVector<QVector<float>> cat_pair_equal_depth;

	QVector<QVector<float>> cat_pair_uncertain_height;
	QVector<QVector<float>> cat_pair_uncertain_medium;
	QVector<QVector<float>> cat_pair_uncertain_depth;
	
	QVector<QVector<float>> cat_word2vec_similarity;

	// cooccurence
	QVector<QVector<int>> cat_cooccur_indicator; // 1 for co-occur

	// connected
	QMap<CatName, QStringList> cat_most_sim_mappings;	
	QMap<QPair<int, int>, QVector<QPair<int, int>>> connected_cat_pair_indices;
	QVector<QPair<QPair<CatName, CatName>, QPair<CatName, CatName>>> connected_cat_pair_names;

	// from user
	QVector<QPair<QPair<CatName, CatName>, Relation>> user_preferences_height;
	QVector<QPair<QPair<CatName, CatName>, Relation>> user_preferences_medium;
	QVector<QPair<QPair<CatName, CatName>, Relation>> user_preferences_depth;

	// all variables
	QVector<QPair<int, int>> all_variables;
// methods
	void init();
	void initCategories();
	void initCatProb();
	void initCatPairwiseProb();
	void completeCatPairwiseProb();
	void initCatSimMatrix();
	void initCatMostSim();
	void initConnectedPairs();
	void initCooccurPairs();
	// for active learning
	void initAllVariables();

	// active learning
	void propagateUserPreference(QVector<QVector<float>> &pair_pref,
		QVector<QVector<float>> &pair_equal,
		QVector<QPair<QPair<CatName, CatName>, Relation>> user_pref);
	

	// assisting methods
	QVector<QPair<QPair<QString, QString>, float>> readCountFromFile(QString path);
	void getInitialProb(QVector<QVector<float>> &cat_pair_pref, QVector<QVector<float>> &cat_pair_equal, QVector<QVector<float>> &cat_pair_uncertain, QString path_pref, QString path_equal);
	// store the common indices of each cat index
	QVector<int> getCommonCatIndices(int index1, int index2, QVector<QVector<float>> matrix);
	void completeEqualProb(QVector<QVector<float>> &cat_pair_equal);
	void completePrefProb(QVector<QVector<float>> &cat_pair_pref, QVector<QVector<float>> cat_pair_equal, QVector<QVector<float>> &cat_pair_uncertain);
	void updatePrefWithEqual(QVector<QVector<float>> &cat_pair_pref, QVector<QVector<float>> cat_pair_equal);
	QVector<int> getKNNCatIndices(CatName cat, int k);
	float getPairPairSim(int i, int j, int k, int m);

	// export
	void exportPairwiseProb(QVector<QVector<float>> cat_pair_prob, QString path);

	// solve quadratic programming
	QVector<float> getPropagatedResults(QVector<QPair<int, int>> &variables, QVector<float> labels,
		QVector<QVector<float>> &pair_info, float lambda);
	QVector<float> getPropagatedResultsPref(QVector<QPair<int, int>> &variables, QVector<float> labels,
		QVector<QVector<float>> &pair_info, float lambda);
	QVector<float> getPropagatedResultsFull(QVector<QPair<int, int>> &variables, QVector<float> labels,
		QVector<QVector<float>> &pair_info, float lambda);
	void updateEqualProbElementwise(QVector<QPair<int, int>> indices, QVector<float> new_equal,
		QVector<QVector<float>> &pair_pref, QVector<QVector<float>>  &pair_equal);
	void updatePrefProbElementwise(QVector<QPair<int, int>> indices, QVector<float> new_pref,
		QVector<QVector<float>> &pair_pref, QVector<QVector<float>> &pair_equal);
	void scalePrefProbToOne(QVector<QVector<float>> &pair_pref);

};

