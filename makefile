CXX=g++
CXX_FLAGS=-shared -fPIC -c -g -Wall -Wextra -std=c++11
TARGETS=FenJiA FenJiABase
LIBs=$(addsuffix .so,$(addprefix ./lib/lib, $(TARGETS)))
LIB=-lm
INCLUDE=./includes
BIN=Main Test

all:$(addprefix bin/, $(BIN)) doc

lib/lib%.so:cpp/%.cpp 
	$(CXX) $(CXX_FLAGS) -o $@ $< $(LIB) -I$(INCLUDE) 
bin/Main:cpp/Main.cpp $(LIBs)
	$(CXX) cpp/Main.cpp $(LIBs) -o $@ -std=c++11 -I$(INCLUDE)
bin/Test:cpp/Test.cpp
	$(CXX) cpp/Test.cpp $(LIBs) -o $@ -std=c++11 -I$(INCLUDE)
clean:
	rm -rf bin/*
