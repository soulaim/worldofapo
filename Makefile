CXXFLAGS = -Wall -O3 -std=c++98 
LDLIBS = -lSDL -lGL -lGLU -lpng
#-lSDL_mixer
CC = g++

DIRS = src src/net src/frustum


obj = $(patsubst %.cpp,%.o, $(foreach dir,$(DIRS), $(wildcard $(dir)/*.cpp)))
dep = $(obj:.o=.d)

.PHONY: all clean

targets = diablo


all: $(targets)

diablo: $(obj)
	g++ $(obj) $(LDLIBS) -o  bin/diablo

$(dep): %.d: %.cpp
	$(CXX) -MT "$(@:.d=.o) $@" -MM $(CXXFLAGS) $< > $@

clean:
	$(RM) $(targets) $(obj) $(dep)

ifneq ($(MAKECMDGOALS),clean)
  include $(dep)
endif

