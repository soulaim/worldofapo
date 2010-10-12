CXXFLAGS = -pedantic -Wall -Werror -O3 -std=c++0x
LDLIBS   = -lSDL -lSDL_mixer -L ./lib/ -lGL -lGLU -lpng -lGLEW
CXX      = g++

DIRS = src src/net src/frustum

target1 = bin/diablo
target2 = bin/editor
target3 = bin/loader_3ds

all: $(target1) $(target2) $(target3)

prof: CXXFLAGS += -pg
prof: LDLIBS += -pg
prof: $(target1)

obj1 = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS) + src/main,   $(wildcard $(dir)/*.cpp)))
obj2 = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS) + src/editor, $(wildcard $(dir)/*.cpp)))
obj3 = $(patsubst %.cpp,%.o, $(foreach dir,      src/loader_3ds, $(wildcard $(dir)/*.cpp)))

dep = $(obj1:.o=.d)
dep += $(obj2:.o=.d)
dep := $(sort $(dep))

.PHONY: all clean

$(target1): $(obj1)
	$(CXX) $^ $(LDLIBS) -o $@

$(target2): $(obj2)
	$(CXX) $^ $(LDLIBS) -o $@

$(target3): $(obj3)
	$(CXX) $^ $(LDLIBS) -o $@

$(dep): %.d: %.cpp
	$(CXX) -MT "$(@:.d=.o) $@" -MM $(CXXFLAGS) $< > $@

clean:
	$(RM) $(target1) $(target2) $(obj1) $(obj2) $(obj3) $(dep)

ifneq ($(MAKECMDGOALS),clean)
  include $(dep)
endif

