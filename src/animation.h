
#include <vector>

class Animation
{
	std::vector<float> rot_x;
	std::vector<float> rot_y;
	std::vector<float> rot_z;
	public:
		void getAnimationState(int, float&, float&, float&);
		void insertAnimationState(float, float, float);
		void setAnimationState(int, float, float, float);
		int getSize();
};

