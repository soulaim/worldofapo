#include "keymanager.h"

using namespace std;

const string KeyManager::KEYFILENAME = "myKeys";

void KeyManager::saveKey(const string& key)
{
    set<string> keys = readKeys();
    keys.insert(key);
    ofstream keyCodes(KEYFILENAME);
    for (set<string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        keyCodes << *iter << endl;
    }
}

set<string> KeyManager::readKeys()
{
    set<string> keys;

	ifstream keyCodes(KEYFILENAME);

    while (keyCodes.good() && !keyCodes.eof())
    {
        string code; keyCodes >> code;
        if (checkValidKey(code))
            keys.insert(code);
    }
    return keys;
}

bool KeyManager::checkValidKey(const string& key)
{
    return (key.size() == 15);
}
