
#ifndef H_MESSAGING_SYSTEM_TYPEDMESSAGE
#define H_MESSAGING_SYSTEM_TYPEDMESSAGE

#include <set>
#include <vector>

template <class T>
class MessagingSystem
{
	public:
	MessagingSystem();
	~MessagingSystem();
	
	static void sendMessage(const T& t);
	static void queueMessage(const T& t);
	static void deliverMessages();
	
	virtual void handle(const T& t) = 0;
	
	private:
	static std::vector<T> messages;
	static std::set<MessagingSystem*> listeners;
};

#include "misc/messaging_system.icc"

#endif
