#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include "socket.h"

#include <vector>
#include <map>

class SocketHandler
{
public:
	struct Connection
	{
		friend class SocketHandler;
		
		private:
			MU_Socket socket;
			std::vector<std::string> msgs;
			
			unsigned out_marker;
			
			bool alive()
			{
				return socket.alive;
			}
			
			std::string write_buffer;
			std::string read_buffer;
			
			Connection& pushToReadBuffer(const std::string& msg, char delimiter = '#');
			
		public:
			Connection();
			
			bool empty();
			
			// read functionality
			bool operator >> (std::string& msg);
			
			// write functionality
			Connection& operator << (const std::string& msg);
			template <class T> Connection& operator << (const T& msg);
	};
	
	SocketHandler();

	bool open(int, const std::string& host, int port);
	bool listen(int port);
	bool accept(int id);
	bool close(int id);

	bool alive(int id);
	
	Connection& getConnection(int id);
	
	// std::vector<std::string>& read(int id);
	
	void tick();
	int size();
	
private:
	int select(fd_set& read_socks, fd_set& write_socks);
	void read_and_write(const fd_set& read_socks, const fd_set& write_socks);

	std::map<int, Connection> sockets;
	MU_Socket listen_socket;
};


template <class T> SocketHandler::Connection& SocketHandler::Connection::operator << (const T& msg)
{
	std::stringstream ss(msg);
	write_buffer += ss.str();
	return *this;
}


#endif

