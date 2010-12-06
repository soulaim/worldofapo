#ifndef RANDOM_H
#define RANDOM_H

float randf(float min, float max)
{
	return rand() * 1.0 / RAND_MAX * (max - min) + min;
}

class RandomMachine
{
	RandomMachine(): seed(1), i(1)
	{
	}
	
public:
	void setSeed(int s)
	{
		seed = s;
		i = 1;
	}
	
	int getInt()
	{
		++i;
		return 6423783 * seed + 26124743 * (seed + i) + 17377287 * i;
	}
	
	int seed;
	int i;
};

#endif

