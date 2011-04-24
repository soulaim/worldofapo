#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include "socket.h"

#include <vector>
#include <map>

namespace Network
{
	class SocketHandler
	{
	public:
		struct Connection
		{
			friend class SocketHandler;
			
			private:
				TCP_Socket socket;
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
				
				bool operator >> (std::string& msg); // read functionality
				Connection& operator << (const std::string& msg); // write functionality
				template <class T> Connection& operator << (const T& msg); // write functionality
		};
		
		SocketHandler();
		
		void setDelimiter(char);
		
		bool open(int, const std::string& host, int port);
		bool listen(int port);
		bool accept(int id);
		bool close(int id);
		bool alive(int id);
		
		Connection& getConnection(int id);
		
		void tick();
		int size();
		
	private:
		
		char delimiter;
		
		int select(fd_set& read_socks, fd_set& write_socks);
		void read_and_write(const fd_set& read_socks, const fd_set& write_socks);
		
		std::map<int, Connection> sockets;
		TCP_Socket listen_socket;
	};
	
	
	template <class T> SocketHandler::Connection& SocketHandler::Connection::operator << (const T& msg)
	{
		std::stringstream ss; ss << msg;
		write_buffer += ss.str();
		return *this;
	}
}

#endif

