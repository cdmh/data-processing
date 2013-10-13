/*
 * BayesianClassifier.h
 *
 *  Created on: Mar 20, 2009
 *      Author: Simon Lavigne-Giroux
 */

#ifndef BAYESIANCLASSIFIER_H_
#define BAYESIANCLASSIFIER_H_

#include "Domain.h"

#include <string>
#include <vector>
#include <map>

/**
 * TrainingData contains discretized values.
 */
typedef std::vector<int> TrainingData;

/**
 * RawTrainingData contains continuous values.
 */
typedef std::vector<float> RawTrainingData;

/**
 * The BayesianClassifier contains the structure to calculate the most probable output for a certain input.
 *
 * The data used to construct a classifier is contained in a simple text file. Floats are used to represent every data. 
 * 
 * For example, the file contains the following values :
 * 
 * 1.0 2.0 1.0
 * 2.0 1.0 1.0
 * 1.0 2.0 0.0
 * 0.0 0.0 0.0
 * 
 * ---------Beware, it won't work if there is an empty line at the end of the file-------
 * 
 * The input or situation data is contained into the n - 1 first columns. In this case, it is the first and second column.
 * The output observed depending on the situation is the last column.
 * 
 * In this example, these data contain situations observed by a Boss in a 2D scroller video game.
 * 
 * - The first column is the relative position of his opponent on the X axis.
 * - The second column is the relative position of his opponent on the Y axis.
 * - The last column (the output) is a boolean representing if the opponent is hurt or not.
 * 
 */
class BayesianClassifier {

private:
	/**
	 * Number of columns in the training data.
	 */
	int numberOfColumns;
	
	/**
	 * Domains for each column of the training data
	 */
	std::vector<Domain> domains;
	
	/**
	 * TrainingData used to calculate the probabilities. It is flushed after the constructor
	 */
	std::vector<TrainingData> data;
	
	/**
	 * The number of training data set.
	 */
	int numberOfTrainingData;
	
	/**
	 * Probabilities of each ouput -> P(Ouput).
	 */
	std::vector<float> probabilitiesOfOutputs;
	
	/**
	 * Probabilities of each input -> P(effectColum:effectValue | lastColumn:causeValue).
	 */
	std::map<unsigned long, float> probabilitiesOfInputs;

	/**
	 * Construct the classifier from the RawTrainingData in the file.
	 *
	 * Beware : The file must not have an empty line at the end.
	 */
	void constructClassifier(std::string filename);

	/**
	 * Calculate the probabilities for each possibility of inputs.
	 */
	void calculateProbabilitiesOfInputs();

	/**
	 * Calculate the probability of P(effectColum:effectValue | lastColumn:causeValue)
	 * It saves data into the variable probabilitiesOfInputs.
	 */
	void calculateProbability(int effectColumn, int effectValue,
					int causeValue);

	/**
	 * Calculate P(Output) of each output.
	 * It saves data into the variable probabilitiesOfOuputs.
	 */
	void calculateProbabilitiesOfOutputs();

	/**
	 * Calculate the map key for each value in the variable probabilitiesOfInputs
	 */
	unsigned long calculateMapKey(int effectColumn, int effectValue, int causeValue);

	/**
	 * Update the output probabilities from a new set of raw training data.
	 */
	void updateOutputProbabilities(int output);

	/**
	 * Update the probabilities after adding one set of training data.
	 */
	void updateProbabilities(TrainingData trainingData);

	/**
	 * Convert a vector<float> into a vector<int> by discretizing the values
	 * using the domain for each column.
	 */
	TrainingData convertRawTrainingData(RawTrainingData floatVector);
	
	/**
	 * Returns the domain of the output column.
	 */
	Domain getOuputDomain();

public:
	/**
	 * BayesianClassifier constructor. It constructs the classifier with raw training data from the file
	 * and uses domains to generate discrete values (TrainingData).
	 *
	 * Beware : The file must not have an empty line at the end.
	 */
	BayesianClassifier(std::string filename, std::vector<Domain> _domains);

	/**
	 * BayesianClassifier constructor. It constructs a classifier with the specified domain.
	 * Raw training data are not given, it is possible to add data after the construction.
	 */
	BayesianClassifier(std::vector<Domain> _domains);

	/**
	 * Calculate the most probable output given this input with this formula :
	 * P(Output | Input) = 1/Z * P(Output) * P(InputValue1 | Ouput) * P(InputValue2 | Ouput) * ...
	 * The output with the highest probability is returned.
	 */
	int calculateOutput(std::vector<float> input);

	/**
	 * Calculate the probability of this output given this input.
 	 * P(Output | Input) = 1/Z * P(Output) * P(InputValue1 | Ouput) * P(InputValue2 | Ouput) * ...
	 */
	float calculateProbabilityOfOutput(RawTrainingData input, float output);

	/**
	 * Add raw training data from a file to adapt the classifier. 
	 * It updates the variables containing the probabilities.
	 *
	 * Beware : The file must not have an empty line at the end.
	 */
	void addRawTrainingData(std::string filename);

	/**
	 * Add one set of raw training data to adapt the classifier
	 * It updates the variables containing the probabilities.
	 */
	void addRawTrainingData(RawTrainingData rawTrainingData);
};

#endif /* BAYESIANCLASSIFIER_H_ */
