#ifndef RANDOM_H
#define RANDOM_H

float randf(float min, float max);

class RandomMachine
{
public:
	RandomMachine(): ans(1573) {
	}

	void setSeed(unsigned s)
	{
		ans = s;
	}

	enum { MAX_RAND = 1 << 16 };

	int getInt()
	{
		ans = (35317u * ans + 13873u) % MAX_RAND;
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

