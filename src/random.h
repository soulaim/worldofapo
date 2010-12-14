#ifndef RANDOM_H
#define RANDOM_H

float randf(float min, float max);

class RandomMachine
{
public:
	RandomMachine(): ans(1)
	{
	}
	
	void setSeed(unsigned s)
	{
		ans = s;
	}

	enum { MAX_RAND = 1 << 24 };

	int getInt()
	{
		ans = (1103515245u * ans + 12345) % MAX_RAND;
		return ans;
	}

	float getFloat()
	{
		return 1.0f * getInt() / MAX_RAND;
	}

	float getFloat(float min, float max)
	{
		return 1.0f * getInt() / MAX_RAND * (max - min) + min;
	}

private:
	unsigned ans;
};

#endif

