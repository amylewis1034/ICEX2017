#include "Utilities.h"

float randRangef(float l, float h)
{
	float r = rand() / float(RAND_MAX);
	return (1.0f - r) * l + r * h;
}

double randRange(double l, double h)
{
	double r = rand() / double(RAND_MAX);
	return (1.0 - r) * l + r * h;
}
