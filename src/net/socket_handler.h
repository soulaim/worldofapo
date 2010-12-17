#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include "socket.h"
#include <vector>
#include <map>

class SocketHandler
{
public:
	SocketHandler();

	bool open(int, const std::string& host, int port);
	bool listen(int port);
	bool accept(int id);
	bool close(int id);

	bool alive(int id);
	bool write(int id, const std::string& msg);
	std::vector<std::string>& read(int id);

	void tick();
	int size();
private:
	int select(fd_set& read_socks, fd_set& write_socks);
	void read_and_write(const fd_set& read_socks, const fd_set& write_socks);

	struct Connection
	{
		MU_Socket socket;
		std::string write_buffer;

		std::string read_buffer;
		std::vector<std::string> msgs;
		int push_message(const std::string& msg);
	};

	std::map<int, Connection> sockets;
	MU_Socket listen_socket;
};

#endif

