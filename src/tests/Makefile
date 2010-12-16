CXXFLAGS = -pedantic -Wall -Werror -Wextra -std=c++0x -O0 -g

targets= \
	matrixtest \
	apomath_test \
	fixedpointtest \
	squareroot_and_crossproduct \
	interpolationtest \
	point_in_line \
	lightshapetest
# socket_test
# btt_test

all: $(targets)

apomath_test: ../apomath.cpp apomath_test.cpp
matrixtest: matrixtest.cpp ../frustum/vec3.cpp ../frustum/matrix4.cpp test.h
point_in_line: point_in_line.cpp ../fixed_point.cpp ../location.cpp ../algorithms.cpp ../frustum/vec3.cpp ../apomath.cpp
fixedpointtest: fixedpointtest.cpp ../apomath.cpp
squareroot_and_crossproduct: squareroot_and_crossproduct.cpp ../fixed_point.cpp ../location.cpp
socket_test: socket_test.cpp ../net/socket.cpp ../net/socket_handler.cpp
btt_test: btt_test.cpp
interpolationtest: interpolationtest.cpp ../frustum/vec3.cpp ../location.cpp ../fixed_point.cpp test.h ../algorithms.cpp ../frustum/vec3.cpp
lightshapetest: lightshapetest.cpp ../algorithms.cpp ../frustum/vec3.cpp test.h ../location.cpp


clean:
	$(RM) $(targets)

