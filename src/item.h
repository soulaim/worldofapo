
#ifndef H_ITEM
#define H_ITEM

class World;
class Unit;

class Item
{
public:
	virtual void onUse(World&, Unit&) = 0;
	
};

#endif
