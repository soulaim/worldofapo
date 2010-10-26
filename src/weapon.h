#ifndef H_WEAPON
#define H_WEAPON

class Weapon
{
public:
	virtual void fire() = 0;
	virtual void tick() = 0;
};

#endif

