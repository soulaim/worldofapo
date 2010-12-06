
#include "random.h"
#include <cstdlib>

float randf(float min, float max)
{
	return rand() * 1.0 / RAND_MAX * (max - min) + min;
}
