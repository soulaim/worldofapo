
#ifndef H_FPS_MANAGER_YEAH
#define H_FPS_MANAGER_YEAH

class FPS_Manager
{
	long long frame_count;
	long long start_time;
	long long desired_fps;

	public:
		FPS_Manager(long long time, long long fps = 25);

		int reset(const long long& time);
		int setStartTime(const long long& time);
		int need_to_draw(const long long& time);
        int insert();
};

#endif

