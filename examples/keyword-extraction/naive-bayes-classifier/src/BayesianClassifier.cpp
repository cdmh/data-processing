/*
 * BayesianClassifier.cpp
 *
 *  Created on: Mar 20, 2009
 *      Author: Simon Lavigne-Giroux
 */

#include "BayesianClassifier.h"
#include <fstream>

// The threshold to get to select whether an output is valid
#define outputProbabilityTreshold 0.003
// There is a minimum denominator value to remove the possibility of INF and NaN
#define minimumDenominatorValue 0.0000000001

/**
 * BayesianClassifier constructor. It constructs the classifier with raw training data from the file
 * and uses domains to generate discrete values (TrainingData).
 *
 * Beware : The file must not have an empty line at the end.
 */
BayesianClassifier::BayesianClassifier(std::string filename,
		std::vector<Domain> _domains) {
	domains = _domains;
	numberOfColumns = _domains.size();
	constructClassifier(filename);
}

/**
 * BayesianClassifier constructor. It constructs a classifier with the specified domain.
 * Raw training data are not given, it is possible to add data after the construction.
 */
BayesianClassifier::BayesianClassifier(std::vector<Domain> _domains) {
	domains = _domains;
	numberOfColumns = _domains.size();
	calculateProbabilitiesOfInputs();
	calculateProbabilitiesOfOutputs();
	numberOfTrainingData = data.size();
	data.clear();
}

/**
 * Construct the classifier from the RawTrainingData in the file.
 *
 * Beware : The file must not have an empty line at the end.
 */
void BayesianClassifier::constructClassifier(std::string filename) {
	std::ifstream inputFile(filename.c_str());

	while (!inputFile.eof()) {
		TrainingData trainingData;
		float value;
		for (int i = 0; i < numberOfColumns; i++) {
			inputFile >> value;
			trainingData.push_back(domains[i].calculateDiscreteValue(value));
		}

		data.push_back(trainingData);
	}

	inputFile.close();

	calculateProbabilitiesOfInputs();
	calculateProbabilitiesOfOutputs();
	numberOfTrainingData = data.size();
	data.clear();
}

/**
 * Calculate the probabilities for each possibility of inputs.
 */
void BayesianClassifier::calculateProbabilitiesOfInputs() {
	for (int i = 0; i < numberOfColumns - 1; i++) {
		for (int j = 0; j < domains[i].getNumberOfValues(); j++) {
			for (int k = 0; k < getOuputDomain().getNumberOfValues(); k++) {
				calculateProbability(i, j, k);
			}
		}
	}
}

/**
 * Calculate the probability of P(effectColum:effectValue | lastColumn:causeValue)
 * It saves data into the variable probabilitiesOfInputs.
 */
void BayesianClassifier::calculateProbability(int effectColumn,
		int effectValue, int causeValue) {
	
	// The numerator is the number of TrainingData with this effectValue given this causeValue
	float numerator = 0.0;
	// The denominator is the number of TrainingData with this causeValue
	float denominator = 0.0;

	//Calculate the numerator and denominator by scanning the TrainingData
	for (unsigned int i = 0; i < data.size(); i++) {
		TrainingData trainingData = data[i];
		if (trainingData[numberOfColumns - 1] == causeValue) {
			denominator++;
			if (trainingData[effectColumn] == effectValue) {
				numerator++;
			}
		}
	}

	float probability = 0.0;
	
	if (denominator != 0) {
		probability = numerator / denominator;
	}
	
	unsigned long key = calculateMapKey(effectColumn, effectValue, causeValue);
	probabilitiesOfInputs.insert(std::pair<unsigned long, float>(key, probability));
}

/**
 * Calculate P(Output) of each output.
 * It saves data into the variable probabilitiesOfOuputs.
 */
void BayesianClassifier::calculateProbabilitiesOfOutputs() {
	for (int i = 0; i < getOuputDomain().getNumberOfValues(); i++) {
		float count = 0.0;
		
		for (unsigned int j = 0; j < data.size(); j++) {
			if (data[j][numberOfColumns - 1] == i) {
				count++;
			}
		}

		if (data.size() != 0) {
			probabilitiesOfOutputs.push_back(count / (float) data.size());
		} else {
			probabilitiesOfOutputs.push_back(0);
		}
	}
}

/**
 * Calculate the map key for each value in the variable probabilitiesOfInputs
 */
unsigned long BayesianClassifier::calculateMapKey(int effectColumn,
		int effectValue, int causeValue) {
	return causeValue * 100000 + effectColumn * 100 + effectValue;
}

/**
 * Calculate the most probable output given this input with this formula :
 * P(Output | Input) = 1/Z * P(Output) * P(InputValue1 | Ouput) * P(InputValue2 | Ouput) * ...
 * The output with the highest probability is returned.
 */
int BayesianClassifier::calculateOutput(std::vector<float> input) {
	float highestProbability = outputProbabilityTreshold;
	int highestOutput = rand() % getOuputDomain().getNumberOfValues();
	unsigned long key = 0;

	for (int i = 0; i < getOuputDomain().getNumberOfValues(); i++) {
		float probability = probabilitiesOfOutputs[i];

		for (unsigned int j = 0; j < input.size(); j++) {
			key = calculateMapKey(j, domains[j].calculateDiscreteValue(input[j]), i);
			probability *= probabilitiesOfInputs[key];
		}

		if (probability > highestProbability) {
			highestProbability = probability;
			highestOutput = i;
		}
	}

	return highestOutput;
}

/**
 * Calculate the probability of this output given this input.
 * P(Output | Input) = 1/Z * P(Output) * P(InputValue1 | Ouput) * P(InputValue2 | Ouput) * ...
 */
float BayesianClassifier::calculateProbabilityOfOutput(std::vector<float> input, float output) {
	unsigned long key = 0;

	std::vector<float> probabilities;

	for(int i = 0; i < getOuputDomain().getNumberOfValues(); i++) {
		float probability = probabilitiesOfOutputs[i];

		for (unsigned int j = 0; j < input.size(); j++) {
			key = calculateMapKey(j,
					domains[j].calculateDiscreteValue(input[j]), i);

			probability *= probabilitiesOfInputs[key];
		}
		probabilities.push_back(probability);
	}

	float sumOfProbabilities = 0.0;
	for(unsigned int i = 0; i < probabilities.size(); i++) {
		sumOfProbabilities += probabilities[i];
	}

	float alpha = 0.0;

	if(sumOfProbabilities > minimumDenominatorValue) {
		alpha = 1.0 / sumOfProbabilities;
	}

	float probability = probabilities[getOuputDomain().calculateDiscreteValue(output)]*alpha;

	if(probability > 1.0) {
		return 1.0;
	} else {
		return probability;
	}
}

/**
 * Add raw training data from a file to adapt the classifier. 
 * It updates the variables containing the probabilities.
 *
 * Beware : The file must not have an empty line at the end.
 */
void BayesianClassifier::addRawTrainingData(std::string filename) {
	std::ifstream inputFile(filename.c_str());

	while (!inputFile.eof()) {
		RawTrainingData rawTrainingData;
		float value;
		for (int i = 0; i < numberOfColumns; i++) {
			inputFile >> value;
			rawTrainingData.push_back(value);
		}
		addRawTrainingData(rawTrainingData);
	}

	inputFile.close();
}

/**
 * Add one set of raw training data to adapt the classifier
 * It updates the variables containing the probabilities.
 */
void BayesianClassifier::addRawTrainingData(RawTrainingData rawTrainingData){
	std::vector<int> trainingData = convertRawTrainingData(rawTrainingData);

	updateProbabilities(trainingData);
	updateOutputProbabilities(domains[numberOfColumns-1].calculateDiscreteValue(rawTrainingData[numberOfColumns - 1]));

	numberOfTrainingData++;
}

/**
 * Convert a vector<float> into a vector<int> by discretizing the values
 * using the domain for each column.
 */
TrainingData BayesianClassifier::convertRawTrainingData(RawTrainingData floatVector) {
	TrainingData trainingData;

	for(unsigned int i = 0; i < floatVector.size(); i++) {
		trainingData.push_back(domains[i].calculateDiscreteValue(floatVector[i]));
	}

	return trainingData;
}

/**
 * Update the output probabilities from a new set of raw training data.
 */
void BayesianClassifier::updateOutputProbabilities(int output){
	float denominator = numberOfTrainingData;

	for (unsigned int i = 0; i < probabilitiesOfOutputs.size(); i++) {
		float numberOfOutput = probabilitiesOfOutputs[i]*denominator;

		if(i == (unsigned int)output) {
			numberOfOutput++;
		}

		probabilitiesOfOutputs[i] = (float) (numberOfOutput/(denominator + 1.0));
	}
}

/**
 * Update the probabilities after adding one set of training data.
 */
void BayesianClassifier::updateProbabilities(TrainingData trainingData){
	//float denominator = probabilitiesOfOutputs[numberOfColumns - 1]*numberOfTrainingData;
	float denominator = probabilitiesOfOutputs[trainingData[numberOfColumns - 1]]*numberOfTrainingData;
	
	for(int i = 0; i < numberOfColumns - 1; i++) {
		for(int j = 0; j < domains[i].getNumberOfValues(); j++) {
			float numerator = probabilitiesOfInputs[calculateMapKey(i, j, trainingData[numberOfColumns - 1])]*denominator;

			if(j == trainingData[i]) {
				numerator++;
			}

			probabilitiesOfInputs[calculateMapKey(i, j, trainingData[numberOfColumns - 1])] = numerator/(denominator + 1.0);
		}
	}
}

/**
 * Returns the domain of the output column.
 */
Domain BayesianClassifier::getOuputDomain() {
	return domains[numberOfColumns - 1];
}

