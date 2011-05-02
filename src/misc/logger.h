
#ifndef H_LOGGER_
#define H_LOGGER_

#include <string>
#include <fstream>
#include <sstream>

class Logger
{
public:
	void close();
	void open(const std::string&);
	void print(const std::string&);
	Logger();
	~Logger();

private:
	Logger& getSingleton();
	void trueOpen(const std::string&);
	void truePrint(const std::string&);
	std::ofstream file;
};


template<typename T> Logger& operator<<(Logger& logger, const T& t)
{
	std::stringstream ss;
	ss << t;
	logger.print(ss.str());
	return logger;
}

#endif

