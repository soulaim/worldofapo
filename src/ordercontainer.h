
#ifndef ORDERCONTAINER_H
#define ORDERCONTAINER_H

#include <string>
#include <vector>

class OrderContainer
{
	std::string work_copy; 
	void pushWorkCopy();
	
	public:
		
		std::vector<std::string> orders;
		
		OrderContainer(){}
		
		void init();
		void insert(std::string);
};


#endif

