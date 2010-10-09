#ifndef H_VIEW_MESSAGE
#define H_VIEW_MESSAGE

#include <string>

class ViewMessage
{
public:
	
	ViewMessage(): msgContent("<empty message>"), startTime(0), endTime(0), scale(1.5f), hilight(false)
	{
	}
	
	ViewMessage(const std::string& msg): msgContent(msg), startTime(0), endTime(0), scale(1.5f), hilight(false)
	{
	}
	
	ViewMessage(const std::string& msg, unsigned time): msgContent(msg), startTime(time), endTime(time + 10000), scale(1.5f), hilight(false)
	{
	}
	
	std::string msgContent;
	unsigned startTime;
	unsigned endTime;
	
	float scale;
	bool hilight;
};

#endif

