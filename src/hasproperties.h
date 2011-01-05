#ifndef H_HASPROPERTIES
#define H_HASPROPERTIES

#include <string>
#include <unordered_map>

class HasProperties
{
public:
	void save(const std::string& file);
	void load(const std::string& file);
	
	int& operator [] (const std::string& a);
	const int& operator [] (const std::string& a) const;

	std::string& operator() (const std::string& a);
	const std::string& operator() (const std::string& a) const;

	void handleCopyOrder(std::stringstream& ss);

	std::string copyOrder() const;

	std::unordered_map<std::string, int> intVals;
	std::unordered_map<std::string, std::string> strVals;
};

#endif

