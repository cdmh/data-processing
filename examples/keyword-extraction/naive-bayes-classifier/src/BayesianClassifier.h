/*
 * BayesianClassifier.h
 *
 *  Created on: Mar 20, 2009
 *      Author: Simon Lavigne-Giroux
 */

#pragma once

#define USE_VECTOR_MAP 1

#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>
#if !USE_VECTOR_MAP
#include <map>
#endif

#include "Domain.h"

class invalid_key_sequence : public std::runtime_error
{
  public:
    invalid_key_sequence() : std::runtime_error("Key sequence is not sequentially incremental")
    { }
};

class overflow_exception : public std::runtime_error
{
    public:
    overflow_exception() : std::runtime_error("Overflow exception")
    { }
};

template<typename T, typename A=std::allocator<T>>
class vector_map : public std::vector<T, A>
{
  public:
    template<class... V>
    void emplace_back(V &&... values)
    {
        vector::emplace_back(std::forward<V>(values)...);
        if (size() > 2  &&  !less(*(rbegin()+1), *rbegin()))
            throw invalid_key_sequence();
    }

    typename T::second_type &operator[](typename T::first_type key)
    {
        return finder(key)->second;
    }

    iterator find(typename T::first_type key) const
    {
        return const_cast<vector_map *>(this)->finder(key);
    }

    bool const is_sorted() const
    {
        return std::is_sorted(
            begin(),
            end(),
            std::bind(&vector_map::less, this, std::placeholders::_1, std::placeholders::_2));
    }

  protected:
    template<class... V>
    iterator emplace(const_iterator, V &&...);

    template<typename InputIterator>
    void     insert(iterator, InputIterator, InputIterator);
    iterator insert(iterator, value_type const &);
    void     insert(iterator, size_type, value_type const &);

	void push_back(value_type &&);
	void push_back(const value_type &);

    iterator finder(typename T::first_type const &key)
    {
        auto it  = begin();
        auto ite = end();
        it = std::lower_bound(it, ite, key, [](T const &lhs, typename T::first_type const &rhs) {return lhs.first < rhs;});
        if (it != ite  &&  !(key < it->first))
            return it;
        return end();
    }

    bool const less(T const &lhs, T const &rhs) const
    {
        return lhs.first < rhs.first;
    }
};

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
class BayesianClassifier
{
  public:
	/**
	 * BayesianClassifier constructor. It constructs the classifier with raw training data from the file
	 * and uses domains to generate discrete values (TrainingData).
	 *
	 * Beware : The file must not have an empty line at the end.
	 */
	BayesianClassifier(std::string filename, std::vector<Domain> const &_domains);

	/**
	 * BayesianClassifier constructor. It constructs a classifier with the specified domain.
	 * Raw training data are not given, it is possible to add data after the construction.
	 */
	BayesianClassifier(std::vector<Domain> const &_domains);

	/**
	 * Calculate the most probable output given this input with this formula :
	 * P(Output | Input) = 1/Z * P(Output) * P(InputValue1 | Ouput) * P(InputValue2 | Ouput) * ...
	 * The output with the highest probability is returned.
	 */
	int const calculateOutput(std::vector<float> const &input);

    /**
     * calculate all possible outputs
     */
    std::vector<std::pair<int, float>> calculatePossibleOutputs(std::vector<float> const &input) const;

	/**
	 * Calculate the probability of this output given this input.
 	 * P(Output | Input) = 1/Z * P(Output) * P(InputValue1 | Ouput) * P(InputValue2 | Ouput) * ...
	 */
	float const calculateProbabilityOfOutput(RawTrainingData const &input, float output);

	/**
	 * Add raw training data from a file to adapt the classifier. 
	 * It updates the variables containing the probabilities.
	 *
	 * Beware : The file must not have an empty line at the end.
	 */
	void addRawTrainingData(std::string const &filename);

	/**
	 * Add one set of raw training data to adapt the classifier
	 * It updates the variables containing the probabilities.
	 */
	void addRawTrainingData(RawTrainingData const &rawTrainingData);

  private:
	/**
	 * Construct the classifier from the RawTrainingData in the file.
	 *
	 * Beware : The file must not have an empty line at the end.
	 */
	void constructClassifier(std::string const &filename);

	/**
	 * Calculate the probabilities for each possibility of inputs.
	 */
	void calculateProbabilitiesOfInputs();
	void calculateProbabilitiesOfInputsWithoutData();

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
	unsigned long const calculateMapKey(int effectColumn, int effectValue, int causeValue) const;

	/**
	 * Update the output probabilities from a new set of raw training data.
	 */
	void updateOutputProbabilities(int output);

	/**
	 * Update the probabilities after adding one set of training data.
	 */
	void updateProbabilities(TrainingData const &trainingData);

	/**
	 * Convert a vector<float> into a vector<int> by discretizing the values
	 * using the domain for each column.
	 */
	TrainingData convertRawTrainingData(RawTrainingData const &floatVector);
	
	/**
	 * Returns the domain of the output column.
	 */
	Domain const &getOutputDomain() const;

  private:
	/**
	 * Number of columns in the training data.
	 */
	size_t numberOfColumns;
	
    // maximum number of values in any domain
    int max_number_of_domain_values;

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
	size_t numberOfTrainingData;
	
	/**
	 * Probabilities of each ouput -> P(Ouput).
	 */
	std::vector<float> probabilitiesOfOutputs;
	
	/**
	 * Probabilities of each input -> P(effectColum:effectValue | lastColumn:causeValue).
	 */
#if USE_VECTOR_MAP
    vector_map<std::pair<unsigned long const, float>> probabilitiesOfInputs;
#else
    std::map<unsigned long const, float> probabilitiesOfInputs;
#endif
};
