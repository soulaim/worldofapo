
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

// this stuff has surely been implemented thousands of times before, but hey.. what the hell..
class MU_Socket
{
	public:
		MU_Socket();
		MU_Socket(const std::string& ip, int port); // calls conn_init()
		
		int socket_init();
		
		int conn_init(const std::string& ip, int port);
		int write(const std::string& msg);
		std::string read();
		
		int init_listener(int port);
		int accept_connection(SocketHandler&);
		
		int setnonblocking();
		
		int readyToRead();
		int push_message(const std::string& msg);
		std::vector<std::string> msgs;
		
		void closeConnection();
		
		int sock;
		char* read_buffer;
		struct sockaddr_in cliAddr, servAddr;
		std::string order;
		std::string write_buffer;
		
		bool alive;
		int last_order;
};

#endif

