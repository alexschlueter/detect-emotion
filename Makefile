FLAGS=-std=c++11
LIBS=$(shell pkg-config --libs opencv)
TARGETS=playone playall keytest

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    FLAGS+=-g
endif

all: $(TARGETS)

$(TARGETS):
	$(CXX) $(FLAGS) -o $@ $@.cpp $(LIBS)

playone: playone.cpp
playall: playall.cpp
keytest: keytest.cpp

clean:
	rm -f $(TARGETS)
