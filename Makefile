CXXFLAGS = -Wall -O3 -std=c++98 
LDLIBS = -lSDL -lSDL_mixer -lGL -lGLU -lpng
#-lSDL_mixer
CC = g++

DIRS = src src/net src/frustum


obj = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS), $(wildcard $(dir)/*.cpp)))
dep = $(obj:.o=.d)

.PHONY: all clean

target = bin/diablo


all: $(target)

$(target): $(obj)
	g++ $(obj) $(LDLIBS) -o $@

$(dep): %.d: %.cpp
	$(CXX) -MT "$(@:.d=.o) $@" -MM $(CXXFLAGS) $< > $@

clean:
	$(RM) $(targets) $(obj) $(dep)

ifneq ($(MAKECMDGOALS),clean)
  include $(dep)
endif

