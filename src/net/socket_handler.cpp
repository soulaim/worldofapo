
#include "socket_handler.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

SocketHandler::SocketHandler(): nextConnection(0)
{
}

int SocketHandler::add_new(int sock, int id)
{
	int socket_id = id;
	if(id == -1)
	{
		socket_id = nextConnection;
		nextConnection++;
	}
	
	sockets[socket_id] = MU_Socket();
	sockets[socket_id].sock = sock;
	sockets[socket_id].alive = true;

	if(!sockets[socket_id].setnonblocking())
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
	for(map<int, MU_Socket>::iterator iter = sockets.begin(); iter != sockets.end(); iter++)
	{
		if(iter->second.alive == false)
			continue;
		
		FD_SET(iter->second.sock, &fd_socks);
		if(iter->second.sock > high)
			high = iter->second.sock;
	}
	
	int count = select(high+1, &fd_socks, (fd_set *) 0, (fd_set *) 0, &timeout);
	return count;
}

void SocketHandler::read_selected()
{
	for(map<int, MU_Socket>::iterator iter = sockets.begin(); iter != sockets.end(); iter++)
	{
		if(FD_ISSET(iter->second.sock, &fd_socks))
		{
			string ans = iter->second.read();
			if(ans.size() == 0)
			{
				cerr << "Socket #" << iter->first << " was marked readable, but still returned empty string? Marking to be erased.." << endl;
				iter->second.alive = false;
			}
			else
			{
				//	cerr << "Socket #" << i << " sent message: " << ans << endl;
				iter->second.push_message(ans);
			}
		}
	}
}

int SocketHandler::get_writable()
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 5000;
	
	int high = 0;
	
	FD_ZERO(&fd_socks);
	for(map<int, MU_Socket>::iterator iter = sockets.begin(); iter != sockets.end(); iter++)
	{
		if(iter->second.alive == false)
			continue;
		
		FD_SET(iter->second.sock, &fd_socks);
		if(iter->second.sock > high)
			high = iter->second.sock;
	}
	
	int count = select(high+1, (fd_set *) 0, &fd_socks, (fd_set *) 0, &timeout);
	return count;
}


void SocketHandler::erase_id(int id)
{
	cerr << "erasing socket.." << endl;
	close(sockets[id].sock);
	sockets.erase(id);
	cerr << "done" << endl;
}

