
#ifndef H_HASPROPERTIES
#define H_HASPROPERTIES

#include <string>
#include <sstream>
#include <fstream>
#include <cassert>

#include <unordered_map>

#include <iostream> // for debug. remember to delete this later.

class HasProperties
{
public:
	
	HasProperties()
	{
		empty = "";
		zero = 0;
	}
	
	void save(const std::string& file)
	{
		std::ofstream itemFile(file.c_str());
		
		for(auto iter = intVals.begin(); iter != intVals.end(); iter++)
		{
			itemFile << "INT " << iter->first << " " << iter->second << std::endl;
		}
		
		for(auto iter = strVals.begin(); iter != strVals.end(); iter++)
		{
			itemFile << "STRING " << iter->first << " " << iter->second << std::endl;
		}
	}
	
	void load(const std::string& file)
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

	void handleCopyOrder(std::stringstream& ss)
	{
		std::string key;
		ss >> key;
		
		if(key != "INT_VALS")
		{
			std::cerr << " " << key;
			ss >> key;
			std::cerr << " " << key << std::endl;
			
			assert(0 && "HasProperties copyOrder FAIL");
		}
		
		// assert(key == "INT_VALS");
		
		while(ss >> key && key != "STR_VALS")
		{
			ss >> intVals[key];
		}
		while(ss >> key && key != "END_VALS")
		{
			ss >> strVals[key];
		}
	}

	std::string copyOrder() const
	{
		std::stringstream msg;
		msg << " INT_VALS";
		for(auto iter = intVals.begin(); iter != intVals.end(); iter++)
		{
			msg << " " << iter->first << " " << iter->second;
		}
		msg << " STR_VALS";
		for(auto iter = strVals.begin(); iter != strVals.end(); iter++)
		{
			msg << " " << iter->first << " " << iter->second;
		}
		msg << " END_VALS";
		return msg.str();
	}
	
	int zero;
	std::string empty;
	std::unordered_map<std::string, int> intVals;
	std::unordered_map<std::string, std::string> strVals;
};

#endif
