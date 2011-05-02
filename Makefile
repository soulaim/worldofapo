DIRS = src src/net src/graphics src/graphics/frustum src/world src/misc src/local_machine src/physics
INCLUDE_DIRS = -I src
OPENMP   = -fopenmp

PACKAGES = sdl gl glu libpng
CXXFLAGS = -pedantic -Wall -Werror -Wextra -fopenmp -lpthread -std=c++0x -O3 `pkg-config --cflags $(PACKAGES)` $(INCLUDE_DIRS)
LDLIBS   = -lSDL_mixer -L ./lib/ -lGLEW `pkg-config --libs $(PACKAGES)`
CXX      = g++

CLIENT = bin/client
EDITOR = bin/editor
SERVER = bin/server

all: $(CLIENT) $(EDITOR) $(SERVER)

debug: CXXFLAGS += -O0 -g
debug: LDLIBS += -g
debug: all

prof: CXXFLAGS += -pg
prof: LDLIBS += -pg
prof: all

obj1 = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS) + src/main,   $(wildcard $(dir)/*.cpp)))
obj2 = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS) + src/editor, $(wildcard $(dir)/*.cpp)))
obj4 = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS) + src/dedicated, $(wildcard $(dir)/*.cpp)))

dep = $(obj1:.o=.d)
dep += $(obj2:.o=.d)
dep := $(sort $(dep))

.PHONY: all clean

$(CLIENT): $(obj1)
	$(CXX) $^ $(LDLIBS) -o $@
	rm -f bin/myKeys

$(EDITOR): $(obj2)
	$(CXX) $^ $(LDLIBS) -o $@

$(SERVER): $(obj4)
	$(CXX) $^ $(LDLIBS) -o $@

$(dep): %.d: %.cpp
	$(CXX) -MT "$(@:.d=.o) $@" -MM $(CXXFLAGS) $< > $@

clean:
	$(RM) $(CLIENT) $(SERVER) $(EDITOR) $(obj1) $(obj2) $(obj4) $(dep)

ifneq ($(MAKECMDGOALS),clean)
  include $(dep)
endif

