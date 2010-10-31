
#ifndef H_WEAPON
#define H_WEAPON

#include <string>
#include <map>
#include <fstream>

#include "fixed_point.h"
#include "location.h"
#include "projectile.h"

class World;
class Unit;

class Item
{
public:
	virtual void onUse() = 0;
	
	void load(std::string file)
	{
		std::ifstream itemFile(file.c_str());
		std::string word1, word2, word3;
		int val;
		
		while(itemFile >> word1)
		{
			if(word1 == "INT")
			{
				itemFile >> word2 >> val;
				intVals[word2] = val;
			}
			else if(word1 == "STRING")
			{
				itemFile >> word2 >> word3;
				strVals[word2] = word3;
			}
		}
	}
	
	std::map<std::string, int> intVals;
	std::map<std::string, std::string> strVals;
};


class Weapon : public Item
{
	World& w;
	Unit& u;
public:
	
	// ALERT: THIS IS HERE ONLY TO KEEP MEDIKIT OPERATIONAL..
	Weapon(World& _w, Unit& _u):
	w(_w),
	u(_u)
	{
	}
	
	// THIS IS THE TRUE WAY TO GO
	Weapon(World& _w, Unit& _u, const std::string& fileName):
	w(_w),
	u(_u)
	{
		load(fileName);
	}
	
	void fire();
	
	void tick()
	{
		if(intVals["CD_LEFT"] > 0)
			--intVals["CD_LEFT"];
	}
	
	void onUse()
	{
		fire();
	}
};

#endif

