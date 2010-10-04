
#ifndef _H_VIEW_MESSAGE
#define _H_VIEW_MESSAGE

#include <string>

class ViewMessage
{
public:
	
	ViewMessage(): msgContent("<empty message>"), startTime(0), endTime(0), scale(1.5f), hilight(false)
	{
	}
	
	ViewMessage(std::string& msg): msgContent(msg), startTime(0), endTime(0), scale(1.5f), hilight(false)
	{
	}
	
	ViewMessage(std::string& msg, unsigned time): msgContent(msg), startTime(time), endTime(time + 5000), scale(1.5f), hilight(false)
	{
	}
	
	std::string msgContent;
	unsigned startTime;
	unsigned endTime;
	
	float scale;
	bool hilight;
};

#endif

