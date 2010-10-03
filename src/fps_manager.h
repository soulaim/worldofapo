
#ifndef H_FPS_MANAGER_YEAH
#define H_FPS_MANAGER_YEAH

class FPS_Manager
{
	int frame_count;
	int start_time;
	int desired_fps;
	
	public:
		FPS_Manager(int time, int fps = 50);
		int reset();
		int setStartTime(int time);
		int insert();
		int need_to_draw(int time);
		// fps_counter.resize(50, 10000);
};

#endif

