all:
	g++ animation.cpp file_io.cpp graphics.cpp image.cpp main.cpp model.cpp -lGL -lGLU -lpng -lSDL
	g++ animation.cpp file_io.cpp graphics.cpp image.cpp main_animator.cpp model.cpp -lGL -lGLU -lpng -lSDL -o animator
	g++ animation.cpp file_io.cpp graphics.cpp image.cpp main_noemj.cpp model.cpp world.cpp unit.cpp -lGL -lGLU -lpng -lSDL -o noemj