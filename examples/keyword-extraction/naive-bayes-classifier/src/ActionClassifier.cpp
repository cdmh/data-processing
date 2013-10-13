/*
 * ActionClassifier.cpp
 *
 *  Created on: Mar 22, 2009
 *      Author: Simon Lavigne-Giroux
 */

#include "ActionClassifier.h"
#include <fstream>

#define probabilityTreshold 0.01

/**
 * Constructor creating Naive Bayes Classifier for each action type.
 */
ActionClassifier::ActionClassifier(std::string filename, std::vector<Domain> _domains) {
	int numberOfActions = _domains[_domains.size() - 1].getNumberOfValues();
	domains = _domains;

	std::vector<Domain> bayesianDomains = _domains;
	bayesianDomains.pop_back();
	
	for(int i = 0; i < numberOfActions; i++) {
		classifiers.push_back(BayesianClassifier(bayesianDomains));
	}

	std::ifstream inputFile(filename.c_str());

	while (!inputFile.eof()) {
		std::vector<float> rawTrainingData;
		float value;
		float action;
		for (unsigned int i = 0; i < bayesianDomains.size(); i++) {
			inputFile >> value;
			rawTrainingData.push_back(value);
		}
		inputFile >> action;

		classifiers[(int) action].addRawTrainingData(rawTrainingData);
	}

	inputFile.close();
}

/**
 * Returns the best action to perform to obtain a certain output.
 */
int ActionClassifier::calculateAction(std::vector<float> input, float output) {
	float highestProbability = probabilityTreshold;
	int action = rand() % domains[domains.size() - 1].getNumberOfValues();

	for(unsigned int i = 0; i < classifiers.size(); i++) {
		float probability = classifiers[i].calculateProbabilityOfOutput(input, output);

		if(probability >= highestProbability) {
			highestProbability = probability;
			action = i;
		}
	}

	return action;
}

/**
 * Add new training data
 */
void ActionClassifier::addRawTrainingData(std::vector<float> input) {
	int action = (int)(input[input.size() - 1]);
	input.pop_back();

	classifiers[action].addRawTrainingData(input);
}

