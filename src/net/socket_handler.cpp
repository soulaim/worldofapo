
#include "socket_handler.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>

using namespace std;

SocketHandler::SocketHandler()
{
}


bool SocketHandler::accept(int id)
{
	if(listen_socket.readyToRead())
	{
		Connection conn;
		listen_socket.accept_connection(conn.socket);
		
		if(conn.alive())
		{
			cerr << "Accepted a new connection." << endl;
			sockets[id] = conn;
			return true;
		}
		else
		{
			cerr << "Error while accepting a connection!" << endl;
			return false;
		}
	}
	return false;
}

bool SocketHandler::listen(int port)
{
	return listen_socket.init_listener(port);
}

bool SocketHandler::open(int id, const std::string& hostname, int port)
{
	Connection conn;
	conn.socket.conn_init(hostname, port);
	if(conn.socket.alive)
	{
		conn.socket.setnonblocking();
		sockets[id] = conn;
		return true;
	}
	else
	{
		return false;
	}
}

bool SocketHandler::alive(int id)
{
	auto iter = sockets.find(id);
	if(iter == sockets.end())
		return false;
	return iter->second.socket.alive;
}


SocketHandler::Connection& SocketHandler::getConnection(int id)
{
	auto iter = sockets.find(id);
	assert(iter != sockets.end() && "SocketHandler: Got a request for a connection ID that does not exist.");
	return iter->second;
}

/*
bool SocketHandler::write(int id, const std::string& msg)
{
	auto iter = sockets.find(id);
	if(iter == sockets.end())
		return false;
	iter->second.write_buffer += msg;
	return true;
}
*/

/*
std::vector<std::string>& SocketHandler::read(int id)
{
	auto iter = sockets.find(id);
	if(iter == sockets.end())
		throw std::string("Trying to read from dead connection");
	return iter->second.msgs;
}
*/

int SocketHandler::select(fd_set& fd_read_socks, fd_set& fd_write_socks)
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	int high = 0;
	
	FD_ZERO(&fd_write_socks);
	FD_ZERO(&fd_read_socks);
	for(auto iter = sockets.begin(); iter != sockets.end(); iter++)
	{
		if(!iter->second.socket.alive)
		{
			continue;
		}

		FD_SET(iter->second.socket.sock, &fd_read_socks);
		FD_SET(iter->second.socket.sock, &fd_write_socks);
		if(iter->second.socket.sock > high)
		{
			high = iter->second.socket.sock;
		}
	}
	
	int count;
	do
	{
		count = ::select(high+1, &fd_read_socks, &fd_write_socks, 0, &timeout);
	}
	while(count < 0 && errno == EINTR);
	return count;
}

void SocketHandler::read_and_write(const fd_set& read_socks, const fd_set& write_socks)
{
	for(auto iter = sockets.begin(); iter != sockets.end(); iter++)
	{
		auto& conn = iter->second;
		if(FD_ISSET(conn.socket.sock, &read_socks))
		{
			const std::string& read = conn.socket.read();
			if(read.empty())
			{
				cerr << "Connection #" << iter->first << " has disconnected." << endl;
				conn.socket.alive = false;
			}
			
			conn.pushToReadBuffer(read);
		}

		if(FD_ISSET(conn.socket.sock, &write_socks) && !conn.write_buffer.empty())
		{
			int sent = conn.socket.write(conn.write_buffer);
			if(sent > 0)
			{
				conn.write_buffer.erase(0, sent);
			}
		}
	}
}

void SocketHandler::tick()
{
	fd_set read_socks;
	fd_set write_socks;
	int count = select(read_socks, write_socks);
	if(count > 0)
	{
		read_and_write(read_socks, write_socks);
	}
}

bool SocketHandler::close(int id)
{
	cerr << "Closing socket " << id << endl;
	auto iter = sockets.find(id);
	if(iter != sockets.end())
	{
		iter->second.socket.closeConnection();
		sockets.erase(iter);
		cerr << "Done closing socket " << id << endl;
		return true;
	}
	else
	{
		cerr << "Socket " << id << " not found!" << endl;
		return false;
	}
}



bool SocketHandler::Connection::empty()
{
	return msgs.empty();
}

SocketHandler::Connection::Connection()
{
	out_marker = 0;
}

bool SocketHandler::Connection::operator >> (std::string& msg)
{
	if(msgs.empty())
		return false;
	
	msg = msgs[out_marker];
	
	if(++out_marker == msgs.size())
	{
		out_marker = 0;
		msgs.clear();
	}
	
	return true;
}

SocketHandler::Connection& SocketHandler::Connection::operator << (const std::string& msg)
{
	write_buffer += msg;
	return *this;
}


SocketHandler::Connection& SocketHandler::Connection::pushToReadBuffer(const std::string& msg, char delimiter)
{
	if(!msg.empty() && !read_buffer.empty() && msg[0] == delimiter)
	{
		msgs.push_back(read_buffer);
		read_buffer = "";
	}
	
	size_t msg_start = 0;
	for(size_t i = 0; i < msg.size(); ++i)
	{
		if(msg[i] == delimiter)
		{
			if(i == msg_start)
			{
				msg_start = i + 1;
				continue;
			}
			
			msgs.push_back(read_buffer + msg.substr(msg_start, i - msg_start));
			read_buffer = "";
			
			msg_start = i + 1;
		}
	}
	
	if(msg_start <= msg.size()-1)
	{
		read_buffer += msg.substr(msg_start);
	}
	
	return *this;
}


