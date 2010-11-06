
#ifndef H_ITEM
#define H_ITEM

#include <string>
#include <fstream>
#include <map>

class HasProperties
{
public:
	
	HasProperties()
	{
		empty = "";
		zero = 0;
	}
	
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
	
	int& operator [] (const std::string& a)
	{
		return intVals[a];
	}
	
	std::string& operator() (const std::string& a)
	{
		return strVals[a];
	}
	
	const int& operator [] (const std::string& a) const
	{
		auto bla = intVals.find(a);
		if(bla != intVals.end())
			return bla->second;
		return zero;
	}
	
	const std::string& operator() (const std::string& a) const
	{
		auto bla = strVals.find(a);
		if(bla != strVals.end())
			return bla->second;
		return empty;
	}
	
	int zero;
	std::string empty;
	std::map<std::string, int> intVals;
	std::map<std::string, std::string> strVals;
};

class Item
{
public:
	virtual void onUse() = 0;
	
};

#endif
