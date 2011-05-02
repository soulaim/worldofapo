

#include "logger.h"

using namespace std;

Logger::Logger()
{
}

Logger::~Logger()
{
	file.close();
}

void Logger::close()
{
	getSingleton().file.close();
}

Logger& Logger::getSingleton()
{
	static Logger log;
	return log;
}

void Logger::open(const string& fileName)
{
	getSingleton().trueOpen(fileName);
}

void Logger::print(const string& msg)
{
	getSingleton().truePrint(msg);
}

void Logger::trueOpen(const string& fileName)
{
	file.open(fileName.c_str());
}

void Logger::truePrint(const string& msg)
{
	file << msg << flush;
}


