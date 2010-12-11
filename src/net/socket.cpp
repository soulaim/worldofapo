
#include "socket.h"
#include "socket_handler.h"
#include <fcntl.h>

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cstring>


const int READ_BUFFER_SIZE = 1024;
char SHARED_READ_BUFFER[READ_BUFFER_SIZE];

using namespace std;

#ifdef _WIN32
namespace
{
class SocketInitializer
{
public:
	SocketInitializer()
	{
		WSADATA wsaData;
		WORD version = MAKEWORD( 2, 2 );

		int error = WSAStartup( version, &wsaData );

		if(error != 0 ||  LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2)
		{
			cerr << "ERROR: winsock initialization failed" << endl;
			WSACleanup();
			exit(-1);
		}

		cerr << "WinSock jee" << endl;
	}
};

SocketInitializer socket_initializer;
};
#endif


MU_Socket::MU_Socket()
{
	alive = true;
}

MU_Socket::MU_Socket(const string& ip, int port)
{
	//	cerr << "Constructing a new socket and connecting.. " << endl;
	alive = true;
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
	timeout.tv_usec = 0;
	
	/* Create a descriptor set containing our server socket.  */
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	do
	{
		rc = select(sock+1, &fds, NULL, NULL, &timeout);
	}
	while(rc < 0 && errno == EINTR);
	
	if(rc == -1)
	{
		perror("select failed");
		return -1;
	}
	
	return rc;
}


int MU_Socket::setnonblocking()
{
	if(!alive)
		return 0;
	cerr << "Setting socket to non blocking state.. " << endl;
	
#ifdef _WIN32
	u_long iMode=1;
	ioctlsocket(sock,FIONBIO,&iMode);
	cerr << "non blocking state change was successful." << endl;
	return 1;
#else
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
#endif
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

int MU_Socket::write(const string& msg)
{
//	cerr << "Writing data to socket: " << msg << endl;
	if(!alive)
	{
		cerr << "TRYING TO WRITE TO A DEAD SOCKET!! NOT A GOOD IDEA MAYBE?? well fuck you user, i'll just not do that :/" << endl;
		return -1;
	}
	
	const char* c_msg = msg.c_str();
	int length_left = msg.size();
	int total_sent = 0;
	
	while(length_left > 0)
	{
		int data_sent;
		
		do
		{
			data_sent = send(sock, c_msg + total_sent, length_left, 0);
		}
		while(data_sent < 0 && errno == EINTR);
		
		if(data_sent == 0)
		{
			cerr << "no data was written to socket?? :O  wtf? gonna try again.." << endl;
		}
		else if(data_sent < 0)
		{
			cerr << "Network write failed :DD" << endl;
			return -1;
		}
		
		length_left -= data_sent;
		total_sent += data_sent;
	}

	return total_sent;
}

string MU_Socket::read()
{
//	cerr << "Reading data from socket.. " << endl;
	SHARED_READ_BUFFER[0] = '\0';
	
	int data_received = 0;
	do
	{
		data_received = recv(sock, SHARED_READ_BUFFER, READ_BUFFER_SIZE - 1, 0 );
	}
	while(data_received < 0 && errno == EINTR);

	if(data_received >= 0)
	{
		SHARED_READ_BUFFER[data_received] = '\0';
	}

	if(data_received <= 0)
	{
		alive = false;
	}
	
//	cerr << "Read: " << SHARED_READ_BUFFER << "\n";
	return string(SHARED_READ_BUFFER);
}

void MU_Socket::closeConnection()
{
#ifndef _WIN32
	close(sock);
#else
	closesocket(sock);
#endif
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
	alive = true;
	
	return 1;
}

void MU_Socket::accept_connection(MU_Socket& socket)
{
//	cerr << "Accepting connection " << endl;

	socket.sock = accept(sock, 0, 0);
	if(socket.sock > 0)
	{
		socket.alive = true;
	}
	else
	{
		cerr << "Accept failed??" << endl;
	}
}

