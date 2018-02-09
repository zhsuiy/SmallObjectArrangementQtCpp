#include "SmallObjectArrange.h"
#include "Utility.h"
#include "stdlib.h"
#include "../3rdparty/include/alglib-3.13.0/src/optimization.h"
#include <QFileDialog>

using namespace alglib;

SmallObjectArrange::SmallObjectArrange()
{
	// examples for using alglib
	//real_2d_array a = "[[ 2.7,-0.7,-0.9, -0.1],[-0.7,2.9,-0.4,-0.8],[-0.9,-0.4,2.6,-0.3],[-0.1,-0.8,-0.3,2.2]]"; // H
	//real_1d_array b = "[-0.8,-0.2,-0.8,-0.2]"; // f
	//real_1d_array s = "[1,1,1,1]";
	////real_2d_array c = "[[1.0,1.0,2.0],[-1.0,2.0,2.0],[2.0,1.0,3.0]]";
	////integer_1d_array ct = "[-1,-1,-1]";
	//real_1d_array bndl = "[0.0,0.0,0.0,0.0]";
	//real_1d_array bndu = "[0.0,1.0,1.0,1.0]"; //"[+INF,+INF]";
	//real_1d_array x;
	//minqpstate state;
	//minqpreport rep;

	//// create solver, set quadratic/linear terms
	//minqpcreate(4, state);
	//minqpsetquadraticterm(state, a);
	//minqpsetlinearterm(state, b);
	////minqpsetlc(state, c, ct);
	//minqpsetbc(state, bndl,bndu);

	//minqpsetscale(state, s);

	//minqpsetalgobleic(state, 0.0, 0.0, 0.0, 0);
	//minqpoptimize(state);
	//minqpresults(state, x, rep);
	//printf("%s\n", x.tostring(4).c_str()); // EXPECTED: [1.500,0.500]
	//printf("%d\n", int(rep.terminationtype)); // EXPECTED: 4

	//minqpsetalgodenseaul(state, 1.0e-9, 1.0e+4, 5);
	//minqpoptimize(state);
	//minqpresults(state, x, rep);
	//printf("%s\n", x.tostring(4).c_str()); // EXPECTED: [1.500,0.500]
	//printf("%d\n", int(rep.terminationtype)); // EXPECTED: 4
	//
	//minqpsetalgoquickqp(state, 0.0, 0.0, 0.0, 0, true);
	//minqpoptimize(state);
	//minqpresults(state, x, rep);
	//printf("%d\n", int(rep.terminationtype)); // EXPECTED: 4
	//printf("%s\n", x.tostring(4).c_str()); // EXPECTED: [2.5,2]
	
	//init();
	// test progpp	
	
}

SmallObjectArrange::~SmallObjectArrange()
{
}

void SmallObjectArrange::InitArranger()
{
	init();
}

void SmallObjectArrange::UpdateUserPreferences(QVector<QPair<QPair<CatName, CatName>, Relation>> height_pref, QVector<QPair<QPair<CatName, CatName>, Relation>> medium_pref, QVector<QPair<QPair<CatName, CatName>, Relation>> depth_pref)
{
	user_preferences_height.append(height_pref);
	user_preferences_medium.append(medium_pref);
	user_preferences_depth.append(depth_pref);
}

void SmallObjectArrange::PropagateUserPreference()
{
	QVector<QPair<QPair<CatName, CatName>, Relation>> pref;
	//pref.push_back(qMakePair(qMakePair(QString("pencil"), QString("lamp")), Greater));
	pref.push_back(qMakePair(qMakePair(QString("pen"),QString("notebook")), Greater));
	pref.push_back(qMakePair(qMakePair(QString("book"), QString("notebook")), Less));
	//pref.push_back(qMakePair(qMakePair(QString("minifigure"), QString("figurine")), Equal));
	//pref.push_back(qMakePair(qMakePair(QString("notebook"), QString("lamp")), Greater));
	propagateUserPreference(cat_pair_depth, cat_pair_equal_depth, pref);
	//propagateUserPreference(cat_pair_height, cat_pair_equal_height, pref);

	exportPairwiseProb(cat_pair_height, QString("./small-object-results/height-pref-propagated.txt"));
	exportPairwiseProb(cat_pair_equal_height, QString("./small-object-results/height-equal-propagated.txt"));
	exportPairwiseProb(cat_pair_depth, QString("./small-object-results/depth-pref-propagated.txt"));
	exportPairwiseProb(cat_pair_equal_depth, QString("./small-object-results/depth-equal-propagated.txt"));
	exportPairwiseProb(cat_pair_medium, QString("./small-object-results/medium-pref-propagated.txt"));
	exportPairwiseProb(cat_pair_equal_medium, QString("./small-object-results/medium-equal-propagated.txt"));
}

void SmallObjectArrange::init()
{
	m_para = Parameter::GetParameterInstance();
	initCategories();
	initCatProb();
	initCatPairwiseProb();
	completeCatPairwiseProb();
	initCatSimMatrix();
	initCatMostSim();
	initCooccurPairs();
	initConnectedPairs();
	
}

void SmallObjectArrange::initCategories()
{
	all_cats = Utility::ParseStringFromFile(QString("./small-object-info/vocab/cat-r30-d10.txt"));
	m_para->DecorationTypes = all_cats;
	for (size_t i = 0; i < all_cats.size(); i++)
	{
		map_cat_index[all_cats[i]] = i;
	}
}

void SmallObjectArrange::initCatProb()
{
	int n = all_cats.size();
	map_cat_prob.clear();
	auto all_cat_prob = Utility::ParseQStrNameAndFloatValue(QString("./config/priorknowledge/cat_prob.txt"));
	for (size_t i = 0; i < n; i++)
	{
		if (all_cat_prob.contains(all_cats[i]))
		{
			map_cat_prob[all_cats[i]] = all_cat_prob[all_cats[i]];
		}
	}
}

void SmallObjectArrange::initCatPairwiseProb()
{	
	getInitialProb(cat_pair_depth, cat_pair_equal_depth, cat_pair_uncertain_depth, "./small-object-info/fixed/depth_front_preferences.txt", "./small-object-info/fixed/depth_equal_preferences.txt");
	getInitialProb(cat_pair_height, cat_pair_equal_height, cat_pair_uncertain_height, "./small-object-info/fixed/height_higher_preferences.txt", "./small-object-info/fixed/height_equal_preferences.txt");
	getInitialProb(cat_pair_medium, cat_pair_equal_medium, cat_pair_uncertain_medium, "./small-object-info/fixed/medium_center_preferences.txt", "./small-object-info/fixed/medium_equal_preferences.txt");
}

void SmallObjectArrange::completeCatPairwiseProb()
{	
	cout << "Complete height equal pairs" << endl;
	completeEqualProb(cat_pair_equal_height);
	cout << "Complete medium equal pairs" << endl;
	completeEqualProb(cat_pair_equal_medium);
	cout << "Complete depth equal pairs" << endl;
	completeEqualProb(cat_pair_equal_depth);

	cout << "Complete height preference pairs" << endl;
	completePrefProb(cat_pair_height, cat_pair_equal_height, cat_pair_uncertain_height);
	cout << "Complete medium preference pairs" << endl;
	completePrefProb(cat_pair_medium, cat_pair_equal_medium, cat_pair_uncertain_medium);
	cout << "Complete depth preference pairs" << endl;
	completePrefProb(cat_pair_depth, cat_pair_equal_depth, cat_pair_uncertain_depth);

	exportPairwiseProb(cat_pair_height, QString("./small-object-results/height-pref-org.txt"));
	exportPairwiseProb(cat_pair_equal_height, QString("./small-object-results/height-equal-org.txt"));
	exportPairwiseProb(cat_pair_depth, QString("./small-object-results/depth-pref-org.txt"));
	exportPairwiseProb(cat_pair_equal_depth, QString("./small-object-results/depth-equal-org.txt"));
	exportPairwiseProb(cat_pair_medium, QString("./small-object-results/medium-pref-org.txt"));
	exportPairwiseProb(cat_pair_equal_medium, QString("./small-object-results/medium-equal-org.txt"));	
}

void SmallObjectArrange::initCatSimMatrix()
{
	cat_word2vec_similarity.clear();
	// init
	if (all_cats.size() == 0)
	{
		cout << "all_cats has not been initialized!" << endl;
		return;
	}
	int n = all_cats.size();
	for (size_t i = 0; i < n; i++)
	{
		cat_word2vec_similarity.push_back(QVector<float>(n));
	}	
	QString path = "./small-object-info/sim/sim-r30-d10.txt";
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split(' ', QString::SkipEmptyParts);
		if (parts.size() < 3) // skip blank line
			continue;
		QString cat1 = parts[0].trimmed();
		QString cat2 = parts[1].trimmed();
		if (map_cat_index.contains(cat1) && map_cat_index.contains(cat2))
		{
			int index1 = map_cat_index[cat1];
			int index2 = map_cat_index[cat2];
			double sim = parts[2].trimmed().toDouble();
			cat_word2vec_similarity[index1][index2] = sim;
			cat_word2vec_similarity[index2][index1] = sim;
		}		
	}
	file->close();
	delete file;

	// same cat
	for (size_t i = 0; i < n; i++)
	{
		cat_word2vec_similarity[i][i] = 1;
	}
}

void SmallObjectArrange::initCatMostSim()
{
	cat_most_sim_mappings.clear();
	// init
	if (all_cats.size() == 0)
	{
		cout << "all_cats has not been initialized!" << endl;
		return;
	}
	int n = all_cats.size();
	
	QString path = "./small-object-info/most-sim-word/most-sim-word-r30-d10.txt";
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split(':', QString::SkipEmptyParts);
		if (parts.size() < 2) // skip blank line
			continue;
		QString pcat = parts[0].trimmed();
		QStringList simcats = parts[1].split(' ', QString::SkipEmptyParts);
		if (all_cats.contains(pcat))
		{
			cat_most_sim_mappings[pcat] = simcats;
		}		
	}
	file->close();
	delete file;
}

void SmallObjectArrange::propagateUserPreference(QVector<QVector<float>> &pair_pref, 
	QVector<QVector<float>> &pair_equal, 
	QVector<QPair<QPair<CatName, CatName>, Relation>> user_pref)
{
	cout << "Propagating user preferences" << endl;
	// index pair of two categories
	QVector<QPair<int, int>> equal_variables, unequal_variables;
	//QVector<>
	QVector<float> unequal_labels,equal_labels;
	for each (auto constraint in user_pref)
	{
		CatName cat1 = constraint.first.first;
		CatName cat2 = constraint.first.second;
		if (!map_cat_index.contains(cat1) || !map_cat_index.contains(cat2))
			continue;
		int catidx1 = map_cat_index[cat1];
		int catidx2 = map_cat_index[cat2];
		if (constraint.second == Relation::Equal)
		{
			if (pair_equal[catidx1][catidx2] == -1)
			{
				cout << "User specifed a pair (" << cat1.toStdString() << "," 
				<< cat2.toStdString() << ") that never co-occurred in the database.\n";
				pair_pref[catidx1][catidx2] = pair_pref[catidx2][catidx1] = 0.0;
				pair_equal[catidx1][catidx2] = pair_equal[catidx2][catidx1] = 1.0;				
			}
			equal_variables.push_back(qMakePair(catidx1, catidx2));
			equal_labels.push_back(1.0);
		}
		else
		{			
			if (pair_pref[catidx1][catidx2] == -1)
			{
				cout << "User specifed a pair that never co-occurred in the database.\n";
				pair_pref[catidx1][catidx2] = pair_pref[catidx2][catidx1] = 0.5;
				pair_equal[catidx1][catidx2] = pair_equal[catidx2][catidx1] = 0.0;
			}
			unequal_variables.push_back(qMakePair(catidx1, catidx2));
			equal_variables.push_back(qMakePair(catidx1, catidx2));
			//variables.push_back(qMakePair(catidx2, catidx1));
			if (constraint.second == Relation::Greater)
			{
				unequal_labels.push_back(1.0);
				//labels.push_back(0.0);
			}				
			else // Less
			{
				unequal_labels.push_back(0.0);
				//labels.push_back(1.0);
			}
			equal_labels.push_back(0.0);
		}
	}

	// propagate E, update E, update P according to E
	if (equal_variables.size() > 0)
	{
		auto new_equal = getPropagatedResults(equal_variables, equal_labels, pair_equal,m_lambda_equal);
		// using new_equal to fill pair_equal, remember to set undefined pair_pref as well
		updateEqualProbElementwise(equal_variables, new_equal, pair_pref, pair_equal);
		// assure that pij + pji + eij = 1
		updatePrefWithEqual(pair_pref, pair_equal);
	}

	if (unequal_variables.size() > 0)
	{
		// scale P to (0-1) temporarily
		scalePrefProbToOne(pair_pref);
		auto new_pref = getPropagatedResults(unequal_variables, unequal_labels, pair_pref, m_lambda_unequal);
		// use new_pref to fill pair_pref, remember to set pair_equal as well
		updatePrefProbElementwise(unequal_variables, new_pref, pair_pref, pair_equal);
		// assure that pij + pji + eij = 1
		updatePrefWithEqual(pair_pref, pair_equal);
	}
}

QVector<float> SmallObjectArrange::getPropagatedResults(QVector<QPair<int, int>> &variables, QVector<float> labels,
	QVector<QVector<float>> &pair_info, float lambda)
{
	// extend variables
	// using current variables to find connected pairs
	// haven't considered the situation that categories in current room
	int start_pos = 0;
	int round = 0;
	int n_labels = labels.size();	
	while (true)
	{
		int curent_n_var = variables.size();
		QVector<QPair<int, int>> new_variables;

		for (size_t i = start_pos; i < variables.size(); i++)
		{
			if (connected_cat_pair_indices.contains(variables[i]))
			{
				auto connected_pairs = connected_cat_pair_indices[variables[i]];
				for each (auto pair in connected_pairs)
				{
					if (!new_variables.contains(pair) && !variables.contains(pair))
						new_variables.push_back(pair);
				}
			}
		}
		start_pos += curent_n_var;

		// delete duplicate variables keep ij, remove ji
		for (size_t k = 0; k < variables.size(); k++)
		{
			auto cur = variables[k];
			int index_of_reverse = new_variables.indexOf(qMakePair(cur.second, cur.first));
			//cout << "k:" << k << "  reverse index:" << index_of_reverse << endl;
			if (index_of_reverse != -1)
			{
				//cout << "Inside k:" << k << "  reverse index:" << index_of_reverse << endl;
				new_variables.removeAt(index_of_reverse);
			}
		}
		variables.append(new_variables);
		round++;
		if (round == m_variable_round_unequal)
		{
			break;
		}
		if (variables.size() == curent_n_var) // no new element
		{
			break;
		}

		cout << "increased " << variables.size() - curent_n_var
			<< " variables" << endl;
	}
	cout << "total variables: " << variables.size() << endl;
	int n = variables.size();
	QVector<float> results;
	// construct quadratic programming problem	
	real_2d_array W, D, Lambda, G;
	real_1d_array bTLambda; // f
	real_1d_array scales;
	real_1d_array bndl;
	real_1d_array bndu;
	//float dlambda = 10;
	W.setlength(n, n);
	D.setlength(n, n);
	Lambda.setlength(n, n);
	G.setlength(n, n);
	bTLambda.setlength(n);
	scales.setlength(n);
	bndl.setlength(n);
	bndu.setlength(n);
	QVector<float> d_rows;
	for (size_t i = 0; i < n; i++)
	{
		float sum = 0.0;
		for (size_t j = 0; j < n; j++)
		{
			auto sim = getPairPairSim(variables[i].first, variables[i].second,
				variables[j].first, variables[j].second);
			W(i, j) = 0.001*exp(10 * sim);
			//W(i, j) = sim == 1 ? 1000 : (sim / (1 - sim))*(sim / (1 - sim));
			//cout << "Similarity: " << W(i, j) << endl;
			D(i, j) = 0.0;
			Lambda(i, j) = 0.0;
			sum += W(i, j);
		}
		d_rows.push_back(sum);
	}
	for (size_t i = 0; i < n; i++)
	{
		D(i, i) = d_rows[i];
		Lambda(i, i) = lambda;
	}
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			G(i, j) = D(i, j) - W(i, j) + Lambda(i, j);
			//cout << "G(i,j): " << i <<"," << j << "\t" << G(i,j) << endl;
		}
	}
	// set box constraints, labeled variables fixed 0 or 1, unlabeled [0,1]
	// labeled
	for (size_t i = 0; i < n_labels; i++)
	{
		bndl(i) = labels[i];
		bndu(i) = labels[i];
	}
	// unlabelled
	for (size_t i = n_labels; i < n; i++)
	{
		bndl(i) = 0.0;
		//float equal_prob = pair_equal[variables[i].first][variables[i].second];		
		//bndu(i) = equal_prob == -1 ? 1.0 : 1 - equal_prob;
		bndu(i) = 1.0;
	}
	// set btlambda and scales
	int n_notindatabase = 0;
	for (size_t i = 0; i < n; i++)
	{
		scales(i) = 1.0;
		int i1 = variables[i].first;
		int i2 = variables[i].second;
		// remember to set pair_pref when values are calculated
		if (pair_info[i1][i2] == -1)
		{
			bTLambda(i) = 0.0;
			n_notindatabase++;
		}
		else
		{
			//cout << "pair_pref[i1][i2] = " << pair_pref[i1][i2] << endl;
			bTLambda(i) = -pair_info[i1][i2] * lambda;
		}
	}
	cout << "Number of not in database: " << n_notindatabase << endl;

	// solve
	real_1d_array x;
	minqpstate state;
	minqpreport rep;

	// create solver, set quadratic/linear terms
	minqpcreate(n, state);
	minqpsetquadraticterm(state, G);
	minqpsetlinearterm(state, bTLambda);
	//minqpsetlc(state, c, ct);
	minqpsetbc(state, bndl, bndu);

	minqpsetscale(state, scales);


	minqpsetalgobleic(state, 0.0, 0.0, 0.0, 0);
	minqpoptimize(state);
	minqpresults(state, x, rep);
	printf("%s\n", x.tostring(4).c_str()); // EXPECTED: [1.500,0.500]
	printf("%d\n", int(rep.terminationtype)); // EXPECTED: 4	

	QString fileName("./small-object-results/propagate-results.txt");
	if (!fileName.isNull())
	{
		QFile file(fileName); // if not exist, create
		file.open(QIODevice::WriteOnly);
		file.close();
		file.open(QIODevice::ReadWrite);
		if (file.isOpen())
		{
			QTextStream txtOutput(&file);
			for (size_t i = 0; i < n; i++)
			{
				auto cat1 = all_cats[variables[i].first];
				auto cat2 = all_cats[variables[i].second];
				txtOutput << cat1 << " " << cat2 << ":" << x[i] << "\n";
			}
		}
		file.close();
	}
	for (size_t i = 0; i < x.length(); i++)
	{
		results.push_back(x[i]);
	}
	return results;
}

void SmallObjectArrange::updateEqualProbElementwise(QVector<QPair<int, int>> indices, QVector<float> new_equal,
	QVector<QVector<float>> &pair_pref, QVector<QVector<float>>  &pair_equal)
{
	int n = indices.size();
	for (size_t i = 0; i < n; i++)
	{
		int i1 = indices[i].first;
		int i2 = indices[i].second;
		// new element, complete corresponding element in preference matrix
		if (pair_equal[i1][i2] == -1)
		{
			pair_pref[i1][i2] = pair_pref[i2][i1] = (1.0 - new_equal[i]) / 2.0;;
		}
		pair_equal[i1][i2] = pair_equal[i2][i1] = new_equal[i];
	}
}

void SmallObjectArrange::updatePrefProbElementwise(QVector<QPair<int, int>> indices, QVector<float> new_pref,
	QVector<QVector<float>> &pair_pref, QVector<QVector<float>> &pair_equal)
{
	int n = indices.size();
	for (size_t i = 0; i < n; i++)
	{
		int i1 = indices[i].first;
		int i2 = indices[i].second;
		// new element, complete corresponding element in equal matrix
		if (pair_pref[i1][i2] == -1)
		{
			pair_equal[i1][i2] = pair_equal[i2][i1] = 0.0;
		}
		pair_pref[i1][i2] = new_pref[i];
		pair_pref[i2][i1] = 1.0 - new_pref[i];
	}
}


void SmallObjectArrange::scalePrefProbToOne(QVector<QVector<float>> &pair_pref)
{
	int n = pair_pref.size();
	for (size_t i = 0; i < n; i++)
	{
		// if j==i, pair_pref[i][j] = 0.5
		for (size_t j = i; j < n; j++)
		{
			if (pair_pref[i][j] != -1)
			{
				//float prob_ineq = 1 - cat_pair_equal[i][j];
				float pij = pair_pref[i][j];
				float pji = pair_pref[j][i];
				if (pij + pji > 0)
				{
					pair_pref[i][j] = pij / (pij + pji);
					pair_pref[j][i] = pji / (pij + pji);
				}
			}
		}
	}
}


QVector<QPair<QPair<QString, QString>, float>> SmallObjectArrange::readCountFromFile(QString path)
{
	QVector<QPair<QPair<QString, QString>, float>> data;
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split(' ', QString::SkipEmptyParts);
		if (parts.size() < 3) // skip blank line
			continue;
		QString cat1 = parts[0].trimmed();
		QString cat2 = parts[1].trimmed();		
		float count = parts[2].trimmed().toFloat();
		QPair<QString, QString> cats(cat1, cat2);
		data.push_back(QPair<QPair<QString, QString>, float>(cats, count));
	}
	return data;
}


void SmallObjectArrange::getInitialProb(QVector<QVector<float>> &cat_pair_pref, QVector<QVector<float>> &cat_pair_equal, QVector<QVector<float>> &cat_pair_uncertain,
	QString path_pref, QString path_equal)
{
	cat_pair_pref.clear();
	cat_pair_equal.clear();

	QVector<QVector<float>> sum_count;
	if (all_cats.size() == 0)
	{
		cout << "all_cats has not been initialized!" << endl;
		return;
	}
	int n = all_cats.size();
	for (size_t i = 0; i < n; i++)
	{		
		cat_pair_pref.push_back(QVector<float>(n,-1));
		cat_pair_equal.push_back(QVector<float>(n,-1));
		cat_pair_uncertain.push_back(QVector<float>(n, -1));
		sum_count.push_back(QVector<float>(n));
	}

	auto pref = readCountFromFile(path_pref);
	auto equal_pref = readCountFromFile(path_equal);
	for (size_t i = 0; i < pref.size(); i++)
	{
		auto cat1 = pref[i].first.first;
		auto cat2 = pref[i].first.second;
		auto count = pref[i].second;
		if (all_cats.contains(cat1) && all_cats.contains(cat2))
		{
			auto ci1 = map_cat_index[cat1];
			auto ci2 = map_cat_index[cat2];
			cat_pair_pref[ci1][ci2] = count;
			if (ci1 != ci2)
			{
				sum_count[ci1][ci2] += count;
				sum_count[ci2][ci1] += count;
			}
			else
				sum_count[ci1][ci2] += count;
			
			// has been seen in the dataset
			cat_pair_uncertain[ci1][ci2] = cat_pair_uncertain[ci2][ci1] = 0;
		}
	}
	for (size_t i = 0; i < equal_pref.size(); i++)
	{
		auto cat1 = equal_pref[i].first.first;
		auto cat2 = equal_pref[i].first.second;
		auto count = equal_pref[i].second;
		if (all_cats.contains(cat1) && all_cats.contains(cat2))
		{
			auto ci1 = map_cat_index[cat1];
			auto ci2 = map_cat_index[cat2];
			cat_pair_equal[ci1][ci2] = cat_pair_equal[ci2][ci1] = count;
			cat_pair_uncertain[ci1][ci2] = cat_pair_uncertain[ci2][ci1] = 0;
			if (ci1 != ci2)
			{
				sum_count[ci1][ci2] += count;
				sum_count[ci2][ci1] += count;
			}
			else
			{
				sum_count[ci1][ci2] += count;
			}
			
		}
	}
	// normalization
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			if (sum_count[i][j] > 0)
			{
				cat_pair_pref[i][j] == -1 ? -1 : cat_pair_pref[i][j] /= sum_count[i][j];				
				cat_pair_equal[i][j] == -1 ? -1 : cat_pair_equal[i][j] /= sum_count[i][j];
				if (cat_pair_pref[i][j] != -1 && cat_pair_pref[j][i] == -1)
					cat_pair_pref[j][i] = 0;
				if (cat_pair_pref[i][j] != -1 && cat_pair_equal[i][j] == -1)
					cat_pair_equal[i][j] = cat_pair_equal[j][i] = 0.0;
			}
		}
	}
}

QVector<int> SmallObjectArrange::getCommonCatIndices(int index1, int index2, QVector<QVector<float>> matrix)
{
	QVector<int> indices;
	int n = matrix.size();	
	for (size_t i = 0; i < n; i++)
	{
		if (matrix[index1][i] != -1 && matrix[index2][i] != -1)
			indices.push_back(i);
	}
	return indices;
}

void SmallObjectArrange::completeEqualProb(QVector<QVector<float>>& cat_pair_equal)
{
	int n = all_cats.size();	
	while (1)
	{
		int newelement = 0;
		QMap<QPair<int, int>, QVector<int>> batch_indices;
		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = i + 1; j < n; j++)
			{
				if (cat_pair_equal[i][j] == -1)
				{
					QPair<int, int> pair(i, j);
					auto indices = getCommonCatIndices(i, j, cat_pair_equal);
					batch_indices[pair] = indices;
				}
			}
		}
		// update
		QMap<QPair<int, int>, QVector<int>>::iterator it;
		for (it = batch_indices.begin(); it != batch_indices.end(); ++it)
		{
			if (it.value().size() > 0) // there is at least one common index
			{
				newelement++;
				float sum_pc_pac_pbc = 0.0;
				float sum_pc = 0.0;
				int a = it.key().first;
				int b = it.key().second;
				auto indices = it.value();
				for (size_t i = 0; i < indices.size(); i++)
				{
					int c = indices[i];
					float pc = map_cat_prob[all_cats[c]];
					auto prob_ac = cat_pair_equal[a][c];
					auto prob_bc = cat_pair_equal[b][c];
					if (isnan(prob_bc) || isnan(prob_ac))
					{
						int aaa = 1;
					}
					sum_pc_pac_pbc += pc*prob_ac*prob_bc;
					sum_pc += pc;
				}
				cat_pair_equal[a][b] = cat_pair_equal[b][a] = sum_pc_pac_pbc / sum_pc;
			}
		}
		cout << newelement << " new pairs completed" << endl;
		if (newelement == 0)
			break;
	}
}

void SmallObjectArrange::completePrefProb(QVector<QVector<float>>& cat_pair_pref, QVector<QVector<float>> cat_pair_equal,
	QVector<QVector<float>> &cat_pair_uncertain)
{
	int n = all_cats.size();
	while (1)
	{
		// update cat_pair_pref according to cat_pair_equal
		updatePrefWithEqual(cat_pair_pref, cat_pair_equal);
		int newelement = 0;
		QMap<QPair<int, int>, QVector<int>> batch_indices;
		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = i + 1; j < n; j++)
			{
				if (cat_pair_pref[i][j] == -1)
				{
					QPair<int, int> pair(i, j);
					auto indices = getCommonCatIndices(i, j, cat_pair_pref);
					batch_indices[pair] = indices;
				}
			}
		}
		// update
		QMap<QPair<int, int>, QVector<int>>::iterator it;
		for (it = batch_indices.begin(); it != batch_indices.end(); ++it)
		{
			if (it.value().size() > 0) // there is at least one common index
			{
				newelement++;
				float sum_ab = 0.0;
				float sum_ba = 0.0;
				float sum_ab_ba = 0.0;
				float sum_ab_uncertain = 0.0;
				int a = it.key().first;
				int b = it.key().second;
				auto indices = it.value();
				for (size_t i = 0; i < indices.size(); i++)
				{
					int c = indices[i];
					float pac = cat_pair_pref[a][c];
					float pca = cat_pair_pref[c][a];
					float pbc = cat_pair_pref[b][c];					
					float pcb = cat_pair_pref[c][b];
					float pc = map_cat_prob[all_cats[c]];
					
					if (pac > pca && pcb > pbc) // A>C && C>B => A>B
					{
						sum_ab += pc*pac*pcb;
						sum_ab_ba += pc*pac*pcb;
						//sum_ab_ba += pc*pca*pbc;
					}
					else if (pca > pac && pbc > pcb) // C>A && B>C => B>A
					{
						sum_ba += pc*pca*pbc;
						//sum_ab_ba += pc*pac*pcb;
						sum_ab_ba += pc*pca*pbc;
					}
					// uncertain
					// count for the cases that c is not between a and b	
					else
					{
						sum_ab_uncertain += pc*pac*pbc;
						sum_ab_uncertain += pc*pca*pcb;
						sum_ab_ba += pc*pac*pbc;
						sum_ab_ba += pc*pca*pcb;						
					}
				}											
				
				if (sum_ab_ba == 0)
				{
					cat_pair_pref[a][b] = cat_pair_pref[b][a] = 0.0;					
				}
				else
				{ 
					cat_pair_pref[a][b] = sum_ab / sum_ab_ba;
					cat_pair_pref[b][a] = sum_ba / sum_ab_ba;
					cat_pair_uncertain[a][b] = cat_pair_uncertain[b][a] = sum_ab_uncertain / sum_ab_ba;
				}				
			}
		}
		cout << newelement << " new pairs completed" << endl;
		if (newelement == 0)
			break;
	}
}

void SmallObjectArrange::updatePrefWithEqual(QVector<QVector<float>>& cat_pair_pref, QVector<QVector<float>> cat_pair_equal)
{
	int n = all_cats.size();
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = i; j < n; j++)
		{
			if (cat_pair_pref[i][j] != -1)
			{
				
				float prob_ineq = 1 - cat_pair_equal[i][j];
				if (i == j)
				{
					cat_pair_pref[i][j] = prob_ineq;
					continue;
				}
				float pij = cat_pair_pref[i][j];
				float pji = cat_pair_pref[j][i];
				if (pij + pji > 0)
				{
					cat_pair_pref[i][j] = prob_ineq*pij / (pij + pji);
					cat_pair_pref[j][i] = prob_ineq*pji / (pij + pji);
				}
				else if (pij + pji == 0)
				{
					cat_pair_pref[i][j] = cat_pair_pref[j][i] = 0;
				}
				else
				{
				}
			}
		}
	}
}

void SmallObjectArrange::initConnectedPairs()
{
	int total_pair = 0;
	connected_cat_pair_indices.clear();
	connected_cat_pair_names.clear();
	int n = all_cats.size();
	if (cat_most_sim_mappings.size() > 0)
	{
		for (size_t i = 0; i < n; i++)
		{
			QVector<int> neighbor_i_k = getKNNCatIndices(all_cats[i], m_neighbor_num);
			for (size_t j = 0; j < n; j++)
			{
				// only consider pairs that ever appeared together
				if (cat_cooccur_indicator[i][j] == 0)
					continue;
				QVector<int> neighbor_j_m = getKNNCatIndices(all_cats[j], m_neighbor_num);
				for (size_t p = 0; p < neighbor_i_k.size(); p++)
				{
					int k = neighbor_i_k[p];
					for (size_t q = 0; q < neighbor_j_m.size(); q++)
					{
						total_pair++;
						int m = neighbor_j_m[q];
						auto pair_pair_sim = getPairPairSim(i, j, k, m);
						if (pair_pair_sim > m_pair_pair_sim_threshold)
						{
							if (!connected_cat_pair_indices.contains(qMakePair(i, j)))
								connected_cat_pair_indices[qMakePair(i, j)] = QVector<QPair<int, int>>();
							connected_cat_pair_indices[qMakePair(i, j)].push_back(qMakePair(k, m));
							connected_cat_pair_names.push_back(qMakePair(qMakePair(all_cats[i], all_cats[j]), qMakePair(all_cats[k], all_cats[m])));
						}						
					}
				}			
			}
		}
		cout << total_pair << endl;
	}
}

void SmallObjectArrange::initCooccurPairs()
{
	// init
	if (all_cats.size() == 0)
	{
		cout << "all_cats has not been initialized!" << endl;
		return;
	}
	cat_cooccur_indicator.clear();
	int n = all_cats.size();
	for (size_t i = 0; i < n; i++)
	{
		cat_cooccur_indicator.push_back(QVector<int>(n, 0));
	}

	QString path = "./small-object-info/fixed/co-occur-pairs.txt";
	QFile *file = new QFile(path);
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
		std::cout << "Can't open file " + path.toStdString() << endl;
	while (!file->atEnd())
	{
		QByteArray line = file->readLine();
		QString str(line);
		QStringList parts = str.split(' ', QString::SkipEmptyParts);
		if (parts.size() < 2) // skip blank line
			continue;
		QString cat1 = parts[0].trimmed();
		QString cat2 = parts[1].trimmed();
		if (map_cat_index.contains(cat1) && map_cat_index.contains(cat2))
		{
			int i = map_cat_index[cat1];
			int j = map_cat_index[cat2];
			cat_cooccur_indicator[i][j] = cat_cooccur_indicator[j][i] = 1;
		}
	}
	file->close();
	delete file;
}

QVector<int> SmallObjectArrange::getKNNCatIndices(CatName cat, int k)
{
	QVector<int> neighbor_indices;
	if (cat_most_sim_mappings.contains(cat))
	{
		auto cats = cat_most_sim_mappings[cat];
		if (k <= cats.size())
		{
			for (size_t i = 0; i < k; i++)
			{
				if (map_cat_index.contains(cats[i]))
					neighbor_indices.push_back(map_cat_index[cats[i]]);
			}
		}
	}
	return neighbor_indices;
}

float SmallObjectArrange::getPairPairSim(int i, int j, int k, int m)
{	
	float sim_ik = cat_word2vec_similarity[i][k];
	float sim_jm = cat_word2vec_similarity[j][m];
	return sim_ik * sim_jm;
}

void SmallObjectArrange::exportPairwiseProb(QVector<QVector<float>> cat_pair_prob, QString path)
{
	int n = cat_pair_prob.size();
	if (!path.isNull())
	{
		QFile file(path); // if not exist, create
		file.open(QIODevice::WriteOnly);
		file.close();
		file.open(QIODevice::ReadWrite);
		if (file.isOpen())
		{
			QTextStream txtOutput(&file);
			for (size_t i = 0; i < n; i++)
			{
				for (size_t j = 0; j < n; j++)
				{
					auto cat1 = all_cats[i];
					auto cat2 = all_cats[j];
					txtOutput << cat1 << " " << cat2 << ":" << cat_pair_prob[i][j] << "\n";
				}
			}
		}
		file.close();
	}
}
