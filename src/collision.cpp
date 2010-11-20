#include "collision.h"

bool Collision::lineBoxRadius(Location b_bot, Location b_top, Location l1, Location l2, const FixedPoint& radius) {
	FixedPoint top_x;
	FixedPoint top_y;
	FixedPoint top_z;
	
	FixedPoint bot_x;
	FixedPoint bot_y;
	FixedPoint bot_z;
	
	if (l1.x < l2.x)
	{
		bot_x = l1.x - radius;
		top_x = l2.x + radius;
	}
	else
	{
		bot_x = l2.x - radius;
		top_x = l1.x + radius;
	}
	if (l1.y < l2.y)
	{
		bot_y = l1.y - radius;
		top_y = l2.y + radius;
	}
	else
	{
		bot_y = l2.y - radius;
		top_y = l1.y + radius;
	}
	if (l1.z < l2.z)
	{
		bot_z = l1.z - radius;
		top_z = l2.z + radius;
	}
	else
	{
		bot_z = l2.z - radius;
		top_z = l1.z + radius;
	}
	
	return boxBox(b_bot, b_top, Location(bot_x, bot_y, bot_z), Location(top_x, top_y, top_z));
}


bool Collision::lineBox(Location b_bot, Location b_top, Location l1, Location l2) {
	FixedPoint top_x;
	FixedPoint top_y;
	FixedPoint top_z;

	FixedPoint bot_x;
	FixedPoint bot_y;
	FixedPoint bot_z;
	
	if (l1.x < l2.x)
	{
		bot_x = l1.x;
		top_x = l2.x;
	}
	else
	{
		bot_x = l2.x;
		top_x = l1.x;
	}
	if (l1.y < l2.y)
	{
		bot_y = l1.y;
		top_y = l2.y;
	}
	else
	{
		bot_y = l2.y;
		top_y = l1.y;
	}
	if (l1.z < l2.z)
	{
		bot_z = l1.z;
		top_z = l2.z;
	}
	else
	{
		bot_z = l2.z;
		top_z = l1.z;
	}
	
	return boxBox(b_bot, b_top, Location(bot_x, bot_y, bot_z), Location(top_x, top_y, top_z));
}

bool Collision::boxBox(Location b1_bot, Location b1_top, Location b2_bot, Location b2_top) {
	return !(b1_top.x < b2_bot.x || b2_top.x < b1_bot.x || b1_top.y < b2_bot.y || b2_top.y < b1_bot.y || b1_top.z < b2_bot.z || b2_top.z < b1_bot.z);
}

