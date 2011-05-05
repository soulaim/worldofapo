#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <string>
#include <sstream>

class PlayerInfo {

public:
	PlayerInfo(): kills(0), deaths(0), last_keyState(0), last_mouseMove(0), last_mousePress(0), connectionState(0) {}
	
	const PlayerInfo& operator = (const PlayerInfo& a)
	{
		name = a.name;
		kills = a.kills;
		deaths = a.deaths;
		key = a.key;
		connectionState = a.connectionState;
		
		return *this;
	}
	
	int getFrameForKeyPress(unsigned current_frame)
	{
		unsigned frameID;
		if(last_keyState < current_frame + 1)
		{
			frameID = current_frame + 1;
			last_keyState = frameID;
			return frameID;
		}
		else
		{
			frameID = ++last_keyState;
			return frameID;
		}
	}
	
	int getFrameForMouseMove(unsigned current_frame)
	{
		unsigned frameID;
		if(last_mouseMove < current_frame + 1)
		{
			frameID = current_frame + 1;
			last_mouseMove = frameID;
			return frameID;
		}
		else
		{
			frameID = ++last_mouseMove;
			return frameID;
		}
	}
	
	int getFrameForMousePress(unsigned current_frame)
	{
		unsigned frameID;
		if(last_mousePress < current_frame + 1)
		{
			frameID = current_frame + 1;
			last_mousePress = frameID;
			return frameID;
		}
		else
		{
			frameID = ++last_mousePress;
			return frameID;
		}
	}
	
	
	void setInputBegin(unsigned frame)
	{
		last_keyState   = frame;
		last_mouseMove  = frame;
		last_mousePress = frame;
	}
	
	std::string name;
	int kills;
	int deaths;
	
	unsigned last_keyState;
	unsigned last_mouseMove;
	unsigned last_mousePress;
	
	
	int connectionState;
	std::string key;
	
	std::string getDescription();
	void readDescription(std::stringstream&);
};

#endif
