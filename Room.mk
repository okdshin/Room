CXX = g++ -std=c++03 -D__STRICT_ANSI__
CXXFLAGS = -Wall -g -D ROOM_UNIT_TEST
INCLUDES = -I/home/ACTIVEDIRECTORY/okdshin/boost_1_55_0 -I/home/ACTIVEDIRECTORY/okdshin/Savage
LIBS = -lGL -lGLEW -lglfw
OBJS = Room.o
PROGRAM = Room.out

all:$(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(INCLUDES) $(LIBS) -o $(PROGRAM)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBS) -c $<

.PHONY: clean
clean:
	rm -f *o $(PROGRAM)
