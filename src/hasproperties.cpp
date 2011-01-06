#include "hasproperties.h"
#include "logger.h"


#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>

void HasProperties::save(const std::string& file)
{
	std::ofstream out(file.c_str());
	
	for(auto iter = intVals.begin(); iter != intVals.end(); iter++)
	{
		out << "INT " << iter->first << " " << iter->second << std::endl;
	}
	
	for(auto iter = strVals.begin(); iter != strVals.end(); iter++)
	{
		out << "STRING " << iter->first << " " << iter->second << std::endl;
	}
}

void HasProperties::load(const std::string& file)
{
	std::string filename1 = file;
	std::string filename2 = file + ".default";
	std::ifstream in1(filename1);
	std::ifstream in2(filename2);

	std::ifstream& in = (in1 ? in1 : in2);
	std::string filename = (in1 ? filename1 : filename2);

	Logger log;
	log << "Loading config file: '" << filename << "'\n";
	
	std::string word1, word2, word3;
	int val;
	while(in >> word1)
	{
		if(word1 == "INT")
		{
			in >> word2 >> val;
			intVals[word2] = val;
		}
		else if(word1 == "STRING")
		{
			in >> word2 >> word3;
			strVals[word2] = word3;
		}
	}
}

int& HasProperties::operator [] (const std::string& a)
{
	return intVals[a];
}

std::string& HasProperties::operator() (const std::string& a)
{
	return strVals[a];
}

const int& HasProperties::operator [] (const std::string& a) const
{
	static const int zero = 0;

	auto bla = intVals.find(a);
	if(bla != intVals.end())
		return bla->second;
	return zero;
}

const std::string& HasProperties::operator() (const std::string& a) const
{
	static const std::string empty = "";

	auto bla = strVals.find(a);
	if(bla != strVals.end())
		return bla->second;
	return empty;
}

void HasProperties::handleCopyOrder(std::stringstream& ss)
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

std::string HasProperties::copyOrder() const
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

