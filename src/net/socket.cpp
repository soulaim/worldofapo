
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

using std::string;
using std::cerr;
using std::endl;


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
			WSACleanup();

			cerr << "ERROR: winsock initialization failed" << endl;
			exit(-1);
		}
	}
};

SocketInitializer socket_initializer;
};
#endif


Network::TCP_Socket::TCP_Socket()
{
	alive = false;
}

Network::TCP_Socket::TCP_Socket(const string& ip, int port)
{
	alive = false;
	conn_init(ip, port);
}

int Network::TCP_Socket::readyToRead()
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
	while(rc < 0 && errno == EINTR); // check if INTERRUPT - This is done to allow profiling

	if(rc == -1)
	{
		perror("select failed");
		return -1;
	}

	return rc;
}


int Network::TCP_Socket::setnonblocking()
{
	if(!alive)
		return 0;

	cerr << "Setting socket to non blocking state.. " << endl;

#ifdef _WIN32
	u_long iMode=1;
	int nodelay = 1;
	ioctlsocket(sock, FIONBIO, &iMode);

	if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)(&nodelay), sizeof(int)) == 0)
	{
		cerr << "Nagle algorithm seems to have been disabled successfully." << endl;
	}
	else
	{
		cerr << "Networking warning: COULD NOT DISABLE NAGLE!" << endl;
	}
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

int Network::TCP_Socket::conn_init(const string& host, int port)
{
	//	cerr << "Connecting to server.. " << endl;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
	{
		cerr << "!!! ERROR: conn_init() failed, construction of socket failed." << endl;
		return 0;
	}

	hostent* serverMachineInfo = gethostbyname(host.c_str());
	if(!serverMachineInfo)
	{
		cerr << "Unable to resolve address of host name \"" << host << "\"" << endl;
		return 0;
	}

	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;

	server.sin_addr.s_addr = ((struct in_addr*)(serverMachineInfo->h_addr))->s_addr;
	server.sin_port = htons(port);

	int conn_result = connect(sock, (struct sockaddr*) (&server), sizeof(server));
	if(conn_result < 0)
	{
		cerr << "!!! ERROR: conn_init() failed, connecting failed." << endl;
		return 0;
	}

	cerr << "Connection successful" << endl;
	alive = true;

	return 1;
}

int Network::TCP_Socket::write(const string& msg)
{
	if(!alive)
	{
		cerr << "WARNING: Attempted to write to an inactive socket." << endl;
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
			cerr << "WARNING: No data was written to socket. Trying again.." << endl;
		}
		else if(data_sent < 0)
		{
			cerr << "ERROR: Writing to socket failed." << endl;
			return -1;
		}

		length_left -= data_sent;
		total_sent += data_sent;
	}

	return total_sent;
}

string Network::TCP_Socket::read()
{
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

	return string(SHARED_READ_BUFFER);
}

void Network::TCP_Socket::closeConnection()
{
	if(alive)
	{
#ifndef _WIN32
		close(sock);
#else
		closesocket(sock);
#endif
		alive = false;
	}
}


int Network::TCP_Socket::init_listener(int port)
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

	while(bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
		cerr << "WARNING: Unable to bind listener to port: " << port << ", attempting to bind to another port" << endl;
		servAddr.sin_port = htons(++port);
	}

	cerr << "Binding to port: " << port << ", great success!" << endl;
	listen(sock, port);
	alive = true;

	return 1;
}

void Network::TCP_Socket::accept_connection(TCP_Socket& socket)
{
	socket.sock = accept(sock, 0, 0);
	if(socket.sock > 0)
	{
		socket.alive = true;
	}
	else
	{
		cerr << "WARNING: Accepting a connection failed." << endl;
	}
}

