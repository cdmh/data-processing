/*
 * Domain.h
 *
 *  Created on: Mar 21, 2009
 *      Author: Simon Lavigne-Giroux
 */

#ifndef DOMAIN_H_
#define DOMAIN_H_

/**
 * A Domain is represented by 2 numbers and the number of values between them. It is used to discretize values.
 * 
 * A Bayes classifier needs discrete values to work properly. For each column, a domain has to be associated. 
 * 
 * For example, the domain for the first column (the relative position of the opponent on the X axis) is defined like this :
 * 
 * Domain(0.0, 2.0, 3)
 * 
 * The minimum value is 0.0 and maximum value is 2.0. The number of values between them are 3. Hence, the conversion is defined like this :
 * 
 * Discrete value = ((value - min) * number of values) / (max - min)
 * 
 * 0 = ((0.5 - 0.0) * 3 / (2.0 - 0.0)
 * 1 = ((0.67 - 0.0) * 3 / (2.0 - 0.0)
 * 1 = ((1.32 - 0.0) * 3 / (2.0 - 0.0)
 * 2 = ((1.34 - 0.0) * 3 / (2.0 - 0.0)
 * 
 * Approximatively, the domains for the first column are these :
 * 
 * [0, 0.666[ = 0
 * [0.666, 1.333[ = 1
 * [1.333, 2.0[ = 2
 * 
 * Changing the number of values between the minimum and maximum can affect greatly the results. Test with multiple values for better results.
 */
class Domain {
private:
	float max;
	float min;
	int numberOfValues;
	
public:
	/**
	 * Constructor with max, min value and number of values
	 */
	Domain(float _min, float _max, int _numberOfValues);
	
	void setMax(float _max);
	void setMin(float _min);
	void setNumberOfValues(int _numberOfValues);

	float getMax();
	float getMin();
	int getNumberOfValues();

	/**
	 * Calculate the discrete value from a float using the max, min and number of values
	 * The minimum discrete value is 0 and maximum is numberOfValues - 1
	 */
	int calculateDiscreteValue(float value);
};

#endif /* DOMAIN_H_ */
