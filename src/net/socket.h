
#ifndef H_SOCKET_MU
#define H_SOCKET_MU

#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>

class SocketHandler;

class MU_Socket
{
	friend class SocketHandler;

	public:
		MU_Socket();
		MU_Socket(const std::string& ip, int port);

		int conn_init(const std::string& ip, int port);
		void closeConnection();

		int write(const std::string& msg);

		int readyToRead();
		std::string read();
		
		int init_listener(int port);
		void accept_connection(MU_Socket&);
		
		int setnonblocking();
		

	private:

		int sock;
		char* read_buffer;
		struct sockaddr_in cliAddr;
		struct sockaddr_in servAddr;

		bool alive;

};

#endif

