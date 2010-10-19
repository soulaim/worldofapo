#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

class Animation
{
	std::vector<float> rot_x;
	std::vector<float> rot_y;
	std::vector<float> rot_z;
	std::vector<size_t> times;
	size_t total_time;

	public:
		Animation();

		void getAnimationState(size_t time, float&, float&, float&) const;
		void insertAnimationState(size_t time, float, float, float);
//		void setAnimationState(size_t time, float, float, float);
		size_t getSize() const;
		size_t totalTime() const;
};

#endif

