/*
 * Domain.cpp
 *
 *  Created on: Mar 21, 2009
 *      Author: Simon Lavigne-Giroux
 */
#include "Domain.h"

Domain::Domain(float _min, float _max, int _numberOfValues) {
	max = _max;
	min = _min;
	numberOfValues = _numberOfValues;
}

void Domain::setMax(float _max) {
	max = _max;
}
void Domain::setMin(float _min) {
	min = _min;
}
void Domain::setNumberOfValues(int _numberOfValues) {
	numberOfValues = _numberOfValues;
}

float Domain::getMax() {
	return max;
}
float Domain::getMin() {
	return min;
}

int Domain::getNumberOfValues() {
	return numberOfValues;
}

int Domain::calculateDiscreteValue(float value) {
	float discreteValue;
	
	discreteValue = (value - min) / (max - min);
	discreteValue *= numberOfValues;
	
	if (discreteValue > max) {
		return numberOfValues - 1;
	} else if (discreteValue < min) {
		return 0;
	}

	return (int) discreteValue;
}
