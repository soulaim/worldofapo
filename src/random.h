#ifndef RANDOM_H
#define RANDOM_H

float randf(float min, float max)
{
	return rand() * 1.0 / RAND_MAX * (max - min) + min;
}

#endif

