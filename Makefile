DIRS = src src/net src/graphics src/graphics/frustum
INCLUDE_DIRS = -I src -I src/graphics

PACKAGES = sdl gl glu libpng
CXXFLAGS = -pedantic -Wall -Werror -Wextra -std=c++0x -O3 `pkg-config --cflags $(PACKAGES)` $(INCLUDE_DIRS)
LDLIBS   = -lSDL_mixer -L ./lib/ -lGLEW `pkg-config --libs $(PACKAGES)`
CXX      = g++

target1 = bin/client
target2 = bin/editor
target3 = bin/loader_3ds
target4 = bin/server

all: $(target1) $(target2) $(target3) $(target4)

debug: CXXFLAGS += -O0 -g
debug: LDLIBS += -g
debug: all

prof: CXXFLAGS += -pg
prof: LDLIBS += -pg
prof: all

obj1 = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS) + src/main,   $(wildcard $(dir)/*.cpp)))
obj2 = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS) + src/editor, $(wildcard $(dir)/*.cpp)))
obj3 = $(patsubst %.cpp,%.o, $(foreach dir,      src/loader_3ds, $(wildcard $(dir)/*.cpp)))
obj4 = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS) + src/dedicated, $(wildcard $(dir)/*.cpp)))

dep = $(obj1:.o=.d)
dep += $(obj2:.o=.d)
dep := $(sort $(dep))

.PHONY: all clean

$(target1): $(obj1)
	$(CXX) $^ $(LDLIBS) -o $@
	rm -f bin/myKeys

$(target2): $(obj2)
	$(CXX) $^ $(LDLIBS) -o $@

$(target3): $(obj3)
	$(CXX) $^ $(LDLIBS) -o $@

$(target4): $(obj4)
	$(CXX) $^ $(LDLIBS) -o $@

$(dep): %.d: %.cpp
	$(CXX) -MT "$(@:.d=.o) $@" -MM $(CXXFLAGS) $< > $@

clean:
	$(RM) $(target1) $(target2) $(target3) $(target4) $(obj1) $(obj2) $(obj3) $(obj4) $(dep)

ifneq ($(MAKECMDGOALS),clean)
  include $(dep)
endif

