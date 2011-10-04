DIRS := src src/net src/graphics src/graphics/menus src/graphics/terrain
DIRS += src/graphics/models src/graphics/skybox src/graphics/particles
DIRS += src/graphics/frustum src/world src/world/objects src/misc
DIRS += src/local_machine src/physics

CLIENT_DIRS := $(DIRS) src/main
SERVER_DIRS := $(DIRS) src/dedicated
EDITOR_DIRS := $(DIRS) src/editor

INCLUDE_DIRS := -I src
OPENMP   := -fopenmp

PACKAGES := sdl gl glu libpng
WARNINGS := -pedantic -Wall -Werror -Wextra

CXXFLAGS := $(WARNINGS) -fopenmp -lpthread -std=c++0x -O3
CXXFLAGS += `pkg-config --cflags $(PACKAGES)` $(INCLUDE_DIRS)

LDLIBS   = -lSDL_mixer -L ./lib/ -lGLEW `pkg-config --libs $(PACKAGES)`
CXX      = g++

CLIENT = bin/client
EDITOR = bin/editor
SERVER = bin/server

SRC := src
OBJ := obj

all: dirs $(CLIENT) $(EDITOR) $(SERVER)

debug: CXXFLAGS += -O0 -g
debug: LDLIBS += -g
debug: all

prof: CXXFLAGS += -pg
prof: LDLIBS += -pg
prof: all

SRC_CLIENT := $(shell find $(CLIENT_DIRS) -maxdepth 1 -name '*.cpp')
SRC_SERVER := $(shell find $(SERVER_DIRS) -maxdepth 1 -name '*.cpp')
SRC_EDITOR := $(shell find $(EDITOR_DIRS) -maxdepth 1 -name '*.cpp')

OBJ_CLIENT := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRC_CLIENT))
OBJ_SERVER := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRC_SERVER))
OBJ_EDITOR := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRC_EDITOR))

DEPFILES := $(OBJ_CLIENT:.o=.d)
DEPFILES += $(OBJ_SERVER:.o=.d)
DEPFILES += $(OBJ_EDITOR:.o=.d)
DEPFILES := $(sort $(DEPFILES))

.PHONY: all clean dirs echo

all: dirs $(CLIENT) $(EDITOR) $(SERVER)

$(CLIENT): $(OBJ_CLIENT)
	$(CXX) $^ $(LDLIBS) -o $@
	rm -f bin/myKeys

$(EDITOR): $(OBJ_EDITOR)
	$(CXX) $^ $(LDLIBS) -o $@

$(SERVER): $(OBJ_SERVER)
	$(CXX) $^ $(LDLIBS) -o $@

$(OBJ)/%.d: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MM -MT "$(@:.d=.o) $@" $< > $@

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPFILES)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) -rf $(CLIENT) $(SERVER) $(EDITOR) $(OBJ)

dirs:
	@mkdir -p $(OBJ)
	@mkdir -p $(patsubst $(SRC)/%, $(OBJ)/%, $(shell find $(CLIENT_DIRS) -type d))
	@mkdir -p $(patsubst $(SRC)/%, $(OBJ)/%, $(shell find $(SERVER_DIRS) -type d))
	@mkdir -p $(patsubst $(SRC)/%, $(OBJ)/%, $(shell find $(EDITOR_DIRS) -type d))

ifneq ($(MAKECMDGOALS),clean)
  -include $(DEPFILES)
endif

