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
	
	int getInt()
	{
		ans = (1103515245u * ans + 12345) % (1 << 24);
		return ans;
	}
private:
	unsigned ans;
};

#endif

