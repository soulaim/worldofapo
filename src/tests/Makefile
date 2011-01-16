CXXFLAGS = -pedantic -Wall -Werror -Wextra -std=c++0x -O0 -g -I ..

VPATH = ..

targets= \
	matrixtest \
	apomath_test \
	fixedpointtest \
	squareroot_and_crossproduct \
	point_in_line \
	lightshapetest \
	interpolationtest \
	datatype_test \
	rotation_test \
	sdlwindow_test \
	glew_test \
	drawing_test \
	shader_test \
	framebuffer_test \
	deferred_test \
	depth_test
# socket_test
# btt_test

all: $(targets)

apomath_test: apomath.cpp apomath_test.cpp
matrixtest: matrixtest.cpp graphics/frustum/vec3.cpp graphics/frustum/matrix4.cpp test.h
point_in_line: point_in_line.cpp fixed_point.cpp location.cpp algorithms.cpp graphics/frustum/vec3.cpp apomath.cpp
fixedpointtest: fixedpointtest.cpp apomath.cpp
squareroot_and_crossproduct: squareroot_and_crossproduct.cpp fixed_point.cpp location.cpp
socket_test: socket_test.cpp net/socket.cpp net/socket_handler.cpp
btt_test: btt_test.cpp
interpolationtest: interpolationtest.cpp graphics/frustum/vec3.cpp location.cpp fixed_point.cpp test.h algorithms.cpp graphics/frustum/vec3.cpp
rotation_test: rotation_test.cpp algorithms.cpp graphics/frustum/vec3.cpp test.h location.cpp
lightshapetest: lightshapetest.cpp algorithms.cpp graphics/frustum/vec3.cpp test.h location.cpp
datatype_test: datatype_test.cpp

sdlwindow_test: CXXFLAGS += `pkg-config --cflags sdl gl`
sdlwindow_test: LDLIBS += `pkg-config --libs sdl gl`
sdlwindow_test: sdlwindow_test.cpp graphics/window.cpp

glew_test: CXXFLAGS += `pkg-config --cflags sdl gl glu`
glew_test: LDLIBS +=  -Wl,--rpath,../../bin/ -L../../bin/ -lGLEW `pkg-config --libs sdl gl glu`
glew_test: glew_test.cpp graphics/opengl.cpp graphics/window.cpp

drawing_test: CXXFLAGS += `pkg-config --cflags sdl gl glu`
drawing_test: LDLIBS += -L ../../lib/ -lGLEW `pkg-config --libs sdl gl glu` -lglut
drawing_test: drawing_test.cpp graphics/opengl.cpp graphics/window.cpp userio.cpp graphics_tests.h

shader_test: CXXFLAGS += `pkg-config --cflags sdl gl glu`
shader_test: LDLIBS += -L ../../lib/ -lGLEW `pkg-config --libs sdl gl glu`
shader_test: shader_test.cpp graphics/opengl.cpp graphics/shader.cpp graphics/window.cpp userio.cpp graphics_tests.h

framebuffer_test: CXXFLAGS += `pkg-config --cflags sdl gl glu libpng`
framebuffer_test: LDLIBS += -L ../../lib/ -lGLEW `pkg-config --libs sdl gl glu libpng`
framebuffer_test: framebuffer_test.cpp graphics/opengl.cpp graphics/shader.cpp graphics/window.cpp userio.cpp graphics/framebuffer.cpp graphics/texturehandler.cpp logger.cpp graphics/image.cpp graphics/loadpng.cpp graphics/graphics_helper.cpp graphics_tests.h

deferred_test: CXXFLAGS += `pkg-config --cflags sdl gl glu libpng`
deferred_test: LDLIBS += -L ../../lib/ -lGLEW `pkg-config --libs sdl gl glu libpng`
deferred_test: deferred_test.cpp graphics/opengl.cpp graphics/shader.cpp graphics/window.cpp userio.cpp graphics/framebuffer.cpp graphics/texturehandler.cpp logger.cpp graphics/image.cpp graphics/loadpng.cpp graphics/graphics_helper.cpp graphics_tests.h

depth_test: CXXFLAGS += `pkg-config --cflags sdl gl glu libpng`
depth_test: LDLIBS += -L ../../lib/ -lGLEW `pkg-config --libs sdl gl glu libpng`
depth_test: depth_test.cpp graphics/opengl.cpp graphics/shader.cpp graphics/window.cpp userio.cpp graphics/framebuffer.cpp graphics/texturehandler.cpp logger.cpp graphics/image.cpp graphics/loadpng.cpp graphics/graphics_helper.cpp graphics_tests.h graphics/basiccamera.cpp graphics/frustum/vec3.cpp graphics/frustum/matrix4.cpp algorithms.cpp location.cpp

clean:
	$(RM) $(targets)

