#ifndef IDGENERATOR_H
#define IDGENERATOR_H

class IDGenerator
{
public:
	IDGenerator():
		next_id(0)
	{
	}

	void setNextID(int id)
	{
		next_id = id;
	}
	int currentID() const
	{
		return next_id;
	}
	int nextID()
	{
		return next_id++;
	}
private:
	int next_id;
};

#endif

