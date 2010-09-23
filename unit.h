
#include <vector>
#include <string>

#include "location.h"
#include "apomath.h"

using namespace std;

class Unit
{
public:
  
  Unit();
  
  Location position;
  int angle; // integer representation of the angle variable.
  int keyState; // what is the unit doing right now, represented as player keyboard input
  
  ApoMath apomath; // megasuper mathematics library! for integer sin/cos :)
		  // ALTHOUGH, THIS SHOULD NOT BE A PART OF UNIT, BUT
		  // DECLARED SOMEWHERE CLOSER TO ROOT. LIKE GAME.H
		  
  void tick();
  float getAngle();
};
