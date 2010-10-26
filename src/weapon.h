#ifndef H_WEAPON
#define H_WEAPON

class Weapon
{
public:
	virtual void shoot() = 0;
	virtual void tick() = 0;
};

#endif

