#ifndef RANDOM_H
#define RANDOM_H

float randf(float min, float max);

class RandomMachine
{
public:
	RandomMachine(): seed(1), i(1)
	{
	}
	
	void setSeed(int s)
	{
		seed = s;
		i = 1;
	}
	
	int getInt()
	{
		++i;
		return (6423783 * seed + 26124743 * (seed + i) + 17377287 * i) >> 4;
	}
private:
	int seed;
	int i;
};

#endif

