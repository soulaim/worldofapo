
#include "ordercontainer.h"

#include <sstream>

using namespace std;

void OrderContainer::init()
{
	return;
}


void OrderContainer::pushWorkCopy()
{
	orders.push_back(work_copy);
	work_copy = "";
	return;
}

void OrderContainer::clear()
{
	orders.clear();
	ids.clear();
	work_copy = "";
}

void OrderContainer::insert(string msg_part)
{
	if(msg_part[0] == '#')
	{
		pushWorkCopy();
		insert(msg_part.substr(1, msg_part.size()-1));
		return;
	}
	
	for(size_t i = 0; i < msg_part.size(); ++i)
	{
		if(msg_part[i] == '#')
		{
			work_copy.append( msg_part.substr( 0, i ) );
			pushWorkCopy();
			
			if(i == (msg_part.size()-1))
				return;
			insert(msg_part.substr(i+1, msg_part.size()-i-1));
			return;
		}
	}
	
	work_copy.append(msg_part);
}

