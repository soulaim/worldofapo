DIRS := src src/net src/graphics src/graphics/menus src/graphics/terrain
DIRS += src/graphics/models src/graphics/skybox src/graphics/particles
DIRS += src/graphics/frustum src/world src/world/objects src/misc
DIRS += src/local_machine src/physics src/world/objects/items src/graphics/hud
DIRS += src/world/logic src/world/level

CLIENT_DIRS := $(DIRS) src/main
SERVER_DIRS := $(DIRS) src/dedicated

INCLUDE_DIRS := -I src
OPENMP   := -fopenmp

PACKAGES := sdl gl glu libpng
WARNINGS := -pedantic -Wall -Werror -Wextra

CXX      := g++
CXXFLAGS := $(WARNINGS) $(OPENMP) -lpthread -std=c++0x -O3 $(INCLUDE_DIRS)
CXXFLAGS += $(shell pkg-config --cflags $(PACKAGES))

LDLIBS   := -lSDL_mixer -L ./lib/ -lGLEW
LDLIBS   += $(shell pkg-config --libs $(PACKAGES))

CLIENT = bin/client
SERVER = bin/server

SRC := src
OBJ := obj
BIN := bin

all: dirs $(CLIENT) $(SERVER)

debug: CXXFLAGS += -O0 -g
debug: LDLIBS += -g
debug: all

prof: CXXFLAGS += -pg
prof: LDLIBS += -pg
prof: all

SRC_CLIENT := $(foreach dir, $(CLIENT_DIRS), $(wildcard $(dir)/*.cpp))
SRC_SERVER := $(foreach dir, $(SERVER_DIRS), $(wildcard $(dir)/*.cpp))

OBJ_CLIENT := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRC_CLIENT))
OBJ_SERVER := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRC_SERVER))

DEPFILES := $(OBJ_CLIENT:.o=.d)
DEPFILES += $(OBJ_SERVER:.o=.d)
DEPFILES := $(sort $(DEPFILES))

.PHONY: all clean dirs echo

all: dirs $(CLIENT) $(SERVER)

$(CLIENT): $(OBJ_CLIENT)
$(SERVER): $(OBJ_SERVER)

$(BIN)/%:
	@echo LINK $@
	@$(CXX) $^ $(LDLIBS) -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	@echo CC $@
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@$(CXX) $(CXXFLAGS) -MM -MT "$(@:.d=.o) $@" $< > $(@:.o=.d)

clean:
	@$(RM) -rf $(CLIENT) $(SERVER) $(OBJ)

dirs:
	@mkdir -p $(OBJ)
	@mkdir -p $(patsubst $(SRC)/%, $(OBJ)/%, $(shell find $(CLIENT_DIRS) -type d))
	@mkdir -p $(patsubst $(SRC)/%, $(OBJ)/%, $(shell find $(SERVER_DIRS) -type d))

ifneq ($(MAKECMDGOALS),clean)
  -include $(DEPFILES)
endif

