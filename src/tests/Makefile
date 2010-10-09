CXXFLAGS = -O0 -g -Wall

all:
	g++ point_in_line.cpp $(CXXFLAGS) ../location.cpp ../algorithms.cpp ../apomath.cpp -o pointlinetest
	g++ fixedpointtest.cpp $(CXXFLAGS) ../apomath.cpp -o fixedpointtest
	g++ squareroot_and_crossproduct.cpp $(CXXFLAGS) ../location.cpp -o squareroot_and_crossproduct
	g++ socket_test.cpp $(CXXFLAGS) ../net/socket.cpp ../net/socket_handler.cpp -o socket_test


