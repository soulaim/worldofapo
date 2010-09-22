
#include <vector>
#include <string>

#include "location.h"

using namespace std;

class Unit
{
public:
  Location location;
  float angle; // remember to fix
  
  void move(float distance);
  // speak();
};