
#include "socket_handler.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

SocketHandler::SocketHandler()
{
}

int SocketHandler::add_new(int sock)
{
//	cerr << "SocketHandler got a new socket, pushing back: " << sock << endl;
	sockets.push_back(MU_Socket());
	sockets.back().sock = sock;
	if(!sockets.back().setnonblocking())
		return 0;
	return 1;
}

int SocketHandler::get_readable()
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 5000;
	
	int high = 0;
	
	FD_ZERO(&fd_socks);
	for(int i=0; i<sockets.size(); i++)
	{
		FD_SET(sockets[i].sock, &fd_socks);
		if(sockets[i].sock > high)
			high = sockets[i].sock;
	}
	
	int count = select(high+1, &fd_socks, (fd_set *) 0, (fd_set *) 0, &timeout);
	return count;
}

vector<int> SocketHandler::read_selected()
{
	vector<int> erased;
	for(int i=0; i<sockets.size(); i++)
	{
		if(FD_ISSET(sockets[i].sock, &fd_socks))
		{
			string ans = sockets[i].read();
			if(ans.size() == 0)
			{
				cerr << "Socket #" << i << " was marked readable, but still returned empty string? Marking to be erased.." << endl;
				erased.push_back(i);
			}
			else
			{
//				cerr << "Socket #" << i << " sent message: " << ans << endl;
				sockets[i].push_message(ans);
			}
		}
	}

	return erased;
}

int SocketHandler::get_writable()
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 5000;
	
	int high = 0;
	
	FD_ZERO(&fd_socks);
	for(int i=0; i<sockets.size(); i++)
	{
		FD_SET(sockets[i].sock, &fd_socks);
		if(sockets[i].sock > high)
			high = sockets[i].sock;
	}
	
	int count = select(high+1, (fd_set *) 0, &fd_socks, (fd_set *) 0, &timeout);
	return count;
}

int SocketHandler::get_errors()
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 5000;
	
	int high = 0;
	
	FD_ZERO(&fd_socks);
	for(int i=0; i<sockets.size(); i++)
	{
		FD_SET(sockets[i].sock, &fd_socks);
		if(sockets[i].sock > high)
			high = sockets[i].sock;
	}
	
	int count = select(high+1, (fd_set *) 0, (fd_set *) 0, &fd_socks, &timeout);
	return count;
}



int SocketHandler::erase_selected()
{
	for(int i=0; i<sockets.size(); i++)
	{
		FD_ISSET(sockets[i].sock, &fd_socks);
		sockets[i] = sockets.back();
		sockets.pop_back();
		--i;
	}

	return 1;
}

