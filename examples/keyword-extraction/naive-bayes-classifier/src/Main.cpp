//============================================================================
// Name        : Naive Bayes Classifier
// Author      : Simon Lavigne-Giroux
// Version     : 1.0
// Copyright   : None
// Description : A flexible Naive Bayes Classifier
//============================================================================

#include "BayesianClassifier.h"
#include "ActionClassifier.h"
#include "Domain.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

void testBayesianClassifier() {
	int errors = 0;
	
	vector<Domain> domains;
	
	// The input domains
	domains.push_back(Domain(0.0, 2.0, 3));
	domains.push_back(Domain(0.0, 2.0, 3));
	// The ouput domain
	domains.push_back(Domain(0.0, 1.0, 2));
	
	BayesianClassifier classifier("/Users/simonlg/Projects/Personnal/NaiveBayesClassifier/Development/trunk/src/data/bayesianClassifierValues.txt", domains);

	// It is possible to not give any data and add it later
	// BayesianClassifier classifier(domains);
	
	vector<float> input1;
	
	input1.push_back(1.0);
	input1.push_back(2.0);
	
	// Calculates the probability of obtaining the output 0.0 given the input (1.0, 2.0)
	if(classifier.calculateProbabilityOfOutput(input1, 0.0) != 0.5) {
		cout << "The probability should be 0.5" << endl;
		errors++;
	}
	
	// Updates the classifier with new values (1.0, 2.0, 0.0)
	classifier.addRawTrainingData("/Users/simonlg/Projects/Personnal/NaiveBayesClassifier/Development/trunk/src/data/newBayesianClassifierValues.txt");
	
	if(classifier.calculateProbabilityOfOutput(input1, 0.0) <= 0.727272 || classifier.calculateProbabilityOfOutput(input1, 0.0) >= 0.727274) {
		cout << "The probability should be 0.727273" << endl;
		errors++;
	}
	
	// The most probable output should be 0.0 given the input (1.0, 2.0)
	if (classifier.calculateOutput(input1) != 0.0) {
		cout << "The output should be 0.0" << endl;
		errors++;
	}
	
	vector<float> newRawTrainingData;
	
	newRawTrainingData.push_back(1.0);
	newRawTrainingData.push_back(2.0);
	newRawTrainingData.push_back(1.0);
	
	classifier.addRawTrainingData(newRawTrainingData);

	if(classifier.calculateProbabilityOfOutput(input1, 0.0) <= 0.49 || classifier.calculateProbabilityOfOutput(input1, 0.0) >= 0.51) {
		cout << "The probability should be 0.5" << endl;
		errors++;
	}
	
	if (errors == 0) {
		cout << "TestBayesianClassifier passed" << endl;
	}
}

void testActionClassifier() {
	int errors = 0;
	
	vector<Domain> domains;
	
	// The input domains
	domains.push_back(Domain(0.0, 2.0, 3));
	domains.push_back(Domain(0.0, 2.0, 3));
	// The ouput domain
	domains.push_back(Domain(0.0, 1.0, 2));	
	// The action domain (there are 3 actions possible : 0, 1 and 2)
	domains.push_back(Domain(0.0, 2.0, 3));
	
	ActionClassifier classifier("/Users/simonlg/Projects/Personnal/NaiveBayesClassifier/Development/trunk/src/data/actionClassifierValues.txt", domains);
	
	vector<float> input1;
	
	input1.push_back(1.0);
	input1.push_back(2.0);
	
	// Calculates the best action to obtain the output 0.0 given the input (1.0, 2.0)
	if(classifier.calculateAction(input1, 0.0) != 0.0) {
		cout << "The best action to obtain an output of 0.0 should be 0.0" << endl;
		errors++;
	}
	
	vector<float> newRawTrainingData;
	
	newRawTrainingData.push_back(1.0);
	newRawTrainingData.push_back(2.0);
	newRawTrainingData.push_back(0.0);
	newRawTrainingData.push_back(2.0);
	
	classifier.addRawTrainingData(newRawTrainingData);
	classifier.addRawTrainingData(newRawTrainingData);
	
	if(classifier.calculateAction(input1, 0.0) != 2.0) {
		cout << "The best action to obtain an output of 0.0 should be 2.0" << endl;
		errors++;
	}
	
	if (errors == 0) {
		cout << "TestActionClassifier passed" << endl;
	}
}

int main() {
	
	testBayesianClassifier();
	testActionClassifier();
	
	return 0;
}
