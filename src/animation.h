
#include <vector>

class Animation
{
	std::vector<float> rot_x;
	std::vector<float> rot_y;
	std::vector<float> rot_z;
	public:
		void getAnimationState(size_t, float&, float&, float&) const;
		void insertAnimationState(float, float, float);
		void setAnimationState(size_t, float, float, float);
		size_t getSize() const;
};

