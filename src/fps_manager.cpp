
#include "fps_manager.h"


FPS_Manager::FPS_Manager(long long time, long long fps)
{
	frame_count = 0;
	start_time  = time;
	desired_fps = fps;
}

int FPS_Manager::reset(const long long& time)
{
	setStartTime(time);
	frame_count = 0;
	return 0;
}

int FPS_Manager::insert()
{
	return ++frame_count;
}

int FPS_Manager::setStartTime(const long long& time)
{
	start_time = time;
	return 1;
}

int FPS_Manager::need_to_draw(const long long& time)
{
	long long real_time = time - start_time;
	
	long long missing_frames = desired_fps * real_time / 1000 - frame_count;
	if(missing_frames > 25)
	{
		reset(time);
		return 0;
	}
	
	if(1000 * frame_count / (1 + real_time) < desired_fps)
		return 1;
	return 0;
}

