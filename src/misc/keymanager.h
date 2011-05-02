#ifndef H_KEYMANAGER_
#define H_KEYMANAGER_

#include <string>
#include <set>
#include <fstream>

class KeyManager
{
private:
	static const std::string KEYFILENAME;
	static bool checkValidKey(const std::string&);
public:
	static void saveKey(const std::string&);
	static std::set<std::string> readKeys();
};

#endif
