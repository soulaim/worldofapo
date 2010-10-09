#include "../net/socket.h"

#include <cassert>
#include <iostream>

using namespace std;

int main()
{
	{
		MU_Socket socket;
		socket.push_message("123#");
		assert(socket.msgs.size() == 1);
		assert(socket.msgs[0] == "123");
		assert(socket.order == "");
	}
	{
		MU_Socket socket;
		socket.push_message("123#456#");
		assert(socket.msgs.size() == 2);
		assert(socket.msgs[0] == "123");
		assert(socket.msgs[1] == "456");
		assert(socket.order == "");
	}
	{
		MU_Socket socket;
		socket.push_message("123#456#789#");
		assert(socket.msgs.size() == 3);
		assert(socket.msgs[0] == "123");
		assert(socket.msgs[1] == "456");
		assert(socket.msgs[2] == "789");
		assert(socket.order == "");
	}
	{
		MU_Socket socket;
		socket.push_message("123");
		assert(socket.msgs.size() == 0);
		assert(socket.order == "123");
	}
	{
		MU_Socket socket;
		socket.push_message("123##456#");
		assert(socket.msgs.size() == 2);
		assert(socket.msgs[0] == "123");
		assert(socket.msgs[1] == "456");
		assert(socket.order == "");
	}
	{
		MU_Socket socket;
		socket.order = "";
		socket.push_message("#123##456###789");
		assert(socket.msgs.size() == 2);
		assert(socket.msgs[0] == "123");
		assert(socket.msgs[1] == "456");
		assert(socket.order == "789");
	}
	{
		MU_Socket socket;
		socket.order = "000";
		socket.push_message("#123##456###789");
		assert(socket.msgs.size() == 3);
		assert(socket.msgs[0] == "000");
		assert(socket.msgs[1] == "123");
		assert(socket.msgs[2] == "456");
		assert(socket.order == "789");
	}
	return 0;
}

