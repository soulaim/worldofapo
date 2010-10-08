
#include "socket.h"
#include "socket_handler.h"
#include <fcntl.h>

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cstring>


#define READ_BUFFER_SIZE 1024

using namespace std;

MU_Socket::MU_Socket()
{
	alive = false;
	last_order = 0;
	socket_init();
}

MU_Socket::MU_Socket(const string& ip, int port)
{
	//	cerr << "Constructing a new socket and connecting.. " << endl;
	socket_init();
	conn_init(ip, port);
}

int MU_Socket::readyToRead()
{
	if(!alive)
		return 0;
	
	fd_set fds;
	struct timeval timeout;
	int rc;
	
	/* Set time limit. */
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;
	
	/* Create a descriptor set containing our server socket.  */
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	rc = select(sock+1, &fds, NULL, NULL, &timeout);
	
	if(rc==-1)
	{
		perror("select failed");
		return -1;
	}
	
	if (rc > 0)
		return 1;
	return 0;
}


int MU_Socket::setnonblocking()
{
	if(!alive)
		return 0;
	
	cerr << "Setting socket to non blocking state.. " << endl;
	int opts;
	
	opts = fcntl(sock,F_GETFL);
	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		return 0;
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0) {
		perror("fcntl(F_SETFL)");
		return 0;
	}
	
	cerr << "non blocking state change was successful." << endl;
	return 1;
}

int MU_Socket::conn_init(const string& host, int port)
{
	//	cerr << "Connecting to server.. " << endl;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
	{
		cerr << "!!! conn_init() failed, construction of socket failed." << endl;
		return 0;
	}
	
	hostent* serverMachineInfo = gethostbyname(host.c_str());
	if(!serverMachineInfo)
	{
		cerr << "Zomg, couldn't find the address of \"" << host << "\"" << endl;
		return 0;
	}
	
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	
	//	server.sin_addr.s_addr = inet_addr(ip.c_str());
	server.sin_addr.s_addr = ((struct in_addr*)(serverMachineInfo->h_addr))->s_addr;
	server.sin_port = htons(port);
	
	int conn_result = connect(sock, (struct sockaddr*) (&server), sizeof(server));
	if(conn_result < 0)
	{
		cerr << "!!! conn_init() failed, connecting failed." << endl;
		return 0;
	}
	
	cerr << "setting client socket state = ALIVE" << endl;
	alive = true;
	
	return 1;
}

int MU_Socket::socket_init()
{
	//	cerr << "Reserving read buffer for socket.. " << endl;
	read_buffer = new char[READ_BUFFER_SIZE];
	last_order = 0;
	return 1;
}

int MU_Socket::write(const string& msg)
{
	if(!alive)
	{
		cerr << "TRYING TO WRITE TO A DEAD SOCKET!! NOT A GOOD IDEA MAYBE?? well fuck you user, i'll just not do that :/" << endl;
		return 0;
	}
	
	const char* c_msg = msg.c_str();
	int total_length = msg.size();
	
	while(total_length != 0)
	{
		int data_sent = send(sock, c_msg, total_length, 0);
		
		// err???  MAYBE SHOULD HANDLE THIS CASE??
		if(data_sent < 0)
		{
			cerr << "network write failed :DD" << endl;
			return 0; // failed
		}
		
		if(data_sent == 0)
		{
			cerr << "no data was written to socket?? :O  wtf? gonna try again.." << endl;
		}
		
		total_length -= data_sent;
		c_msg += data_sent;
	}
	
	return 1;
}

string MU_Socket::read()
{
	//	cerr << "Reading data from socket.. " << endl;
	int data_received = 0;
	read_buffer[0] = '\0';
	
	data_received = recv(sock, read_buffer, READ_BUFFER_SIZE - 1, 0 );
	read_buffer[data_received] = '\0';
	
	if(data_received == 0)
		alive = false;
	
	return string(read_buffer);
}

int MU_Socket::push_message(const string& msg)
{
	
	int last_break = -1;
	for(unsigned i=0; i<msg.size(); i++)
	{
		if(msg[i] == '#')
		{
			if(last_break+1 > i-1)
			{
				last_break = i;
				continue;
			}
			
			if(last_break == -1)
			{
				order.append(msg.substr(last_break+1, i - (last_break+1) ) );
				msgs.push_back(order);
				order = "";
			}
			else
			{
				msgs.push_back(msg.substr(last_break+1, i - (last_break+1) ));
			}
			
			last_break = i;
		}
	}
	
	if(last_break+1 <= msg.size()-1)
		order.append(msg.substr(last_break+1, msg.size() - (last_break+1) ));
	
	return msgs.size();
}


void MU_Socket::closeConnection()
{
	close(sock);
}


int MU_Socket::init_listener(int port)
{
	cerr << "Attempting to listen to port " << port << endl;
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("cannot open socket for listening");
		return 0;
	}
	
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);
	
	if(bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr))<0)
	{
		cerr << "Unable to bind listener to port: " << port << endl << "exiting.." << endl;
		return 0;
	}
	
	cerr << "great success!" << endl;
	listen(sock, port);
	
	return 1;
}

int MU_Socket::accept_connection(SocketHandler& handler)
{
	//	cerr << "Accepting connection.. " << endl;
	
	int new_socket = accept(sock, 0, 0);
	if(new_socket < 0)
	{
		cerr << "Error while accepting a connection!" << endl;
		return 0;
	}
	else
		cerr << "Accepted a new connection :D trololol :D" << endl;
	
	handler.add_new(new_socket);
	return 1;
}

