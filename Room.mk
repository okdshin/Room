CXX = g++ -std=c++03 -D__STRICT_ANSI__ -O3
CXXFLAGS = -Wall -g -D ROOM_UNIT_TEST
INCLUDES = -I/home/okada/Riftia/Savage
LIBS = -lGL -lGLEW -lglfw -L/home/okada/Riftia/Savage/savage/model/tinyobjloader -ltinyobjloader
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
