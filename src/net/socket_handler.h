
#ifndef H_SOCKET_HANDLER
#define H_SOCKET_HANDLER

#include "socket.h"
#include <vector>
#include <map>

class SocketHandler
{
	public:
	SocketHandler();

	int add_new(int sock);
	int get_readable();
	int get_writable();
	int get_errors();

	void read_selected();
	void erase_id(int id);
	
	std::map<int, MU_Socket> sockets;
	fd_set fd_socks;
	int nextConnection;
};

#endif

