
#include "fps_manager.h"


FPS_Manager::FPS_Manager(int time, int fps)
{
	frame_count = 0;
	start_time  = time;
	desired_fps = fps;
}

int FPS_Manager::reset()
{
	frame_count = 0;
	return 0;
}

int FPS_Manager::insert()
{
	return ++frame_count;
}

int FPS_Manager::setStartTime(int time)
{
	start_time = time;
	return 1;
}

int FPS_Manager::need_to_draw(int time)
{
	int real_time = time - start_time;
	if(1000 * frame_count / (1 + real_time) < desired_fps)
		return 1;
	return 0;
}

