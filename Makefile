all:
	g++ animation.cpp file_io.cpp graphics.cpp image.cpp main.cpp model.cpp world.cpp unit.cpp apomath.cpp userio.cpp game.cpp ordercontainer.cpp net/socket.cpp net/socket_handler.cpp -lGL -lGLU -lpng -lSDL -o menu
	g++ animation.cpp file_io.cpp graphics.cpp image.cpp main_animator.cpp model.cpp -lGL -lGLU -lpng -lSDL -o animator
	g++ animation.cpp file_io.cpp graphics.cpp image.cpp main_noemj.cpp model.cpp world.cpp unit.cpp apomath.cpp userio.cpp -lGL -lGLU -lpng -lSDL -o noemj