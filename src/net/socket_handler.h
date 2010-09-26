
#ifndef H_SOCKET_HANDLER
#define H_SOCKET_HANDLER

#include "socket.h"
#include <vector>

class SocketHandler
{
	public:
	SocketHandler();

	int add_new(int sock);
	int get_readable();
	int get_writable();
	int get_errors();

	std::vector<int> read_selected();
	int erase_selected();
	
	std::vector<MU_Socket> sockets;
	fd_set fd_socks;
};

#endif

