/*
 * ActionClassifier.h
 *
 *  Created on: Mar 22, 2009
 *      Author: Simon Lavigne-Giroux
 */
#ifndef ACTIONCLASSIFIER_H_
#define ACTIONCLASSIFIER_H_

#include "BayesianClassifier.h"
#include "Domain.h"

#include <vector>

/**
 * The ActionClassifier is capable of calculating the best action to perform to obtain a certain output.
 * It is possible to add new raw data at runtime and have a better probabilistic classifier.
 * 
 * It uses the BayesianClassifier but it has 1 more column. The data is still contained in a simple text file.
 * 
 * For example, a file contains the following values :
 * 
 * 1.0 2.0 1.0 1.0
 * 2.0 1.0 1.0 1.0
 * 1.0 2.0 0.0 0.0
 * 0.0 0.0 0.0 0.0
 * 
 * ---------Beware, it won't work if there is an empty line at the end of the file-------
 * 
 * The input or situation data is contained into the n - 2 first columns. In this case, it is the first and second column.
 * The output observed depending on the situation is the n - 1 column or in this case, the third column. 
 * The action done in this situation (input) obtaining the output is the last column. 
 * 
 * In my example, these data contain situations observed by a Boss in a 2D scroller video game.
 * 
 * - The first column is the relative position of his opponent on the X axis.
 * - The second column is the relative position of his opponent on the Y axis.
 * - The third column (the output) is a boolean representing if the opponent is hurt or not.
 * - The last column (the action) is a number representing the action done by the Boss. A C++ enumeration is recommended.
 */
class ActionClassifier {

private:
	
	/**
	 * A BayesianClassifier for each action type.
	 */
	std::vector<BayesianClassifier> classifiers;
	
	/**
	 * Domains of each column (Input, Ouput and Action domain).
	 */
	std::vector<Domain> domains;
	
public:

	/**
	 * Constructor creating Naive Bayes Classifier for each action type.
	 */
	ActionClassifier(std::string filename, std::vector<Domain> _domains);

	/**
	 * Returns the best action to perform to obtain a certain output.
	 */
	int calculateAction(std::vector<float> input, float output);

	/**
	 * Add new training data
	 */
	void addRawTrainingData(std::vector<float> input);
};

#endif /* ACTIONCLASSIFIER_H_ */
