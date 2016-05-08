FLAGS=-g -std=c++11
LIBS=$(shell pkg-config --libs opencv)
all:
	c++ $(FLAGS) -o main main.cpp $(LIBS)
