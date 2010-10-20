CXXFLAGS = -O0 -g -Wall

all:
	g++ matrixtest.cpp $(CXXFLAGS) ../frustum/Vec3.cpp ../frustum/matrix4.cpp -o matrixtest
	g++ point_in_line.cpp $(CXXFLAGS) ../fixed_point.cpp ../location.cpp ../algorithms.cpp ../apomath.cpp -o pointlinetest
	g++ fixedpointtest.cpp $(CXXFLAGS) ../apomath.cpp -o fixedpointtest
	g++ squareroot_and_crossproduct.cpp $(CXXFLAGS) ../fixed_point.cpp ../location.cpp -o squareroot_and_crossproduct
#	g++ socket_test.cpp $(CXXFLAGS) ../net/socket.cpp ../net/socket_handler.cpp -o socket_test


