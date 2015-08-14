CXX=g++
CXX_FLAGS=-shared -fPIC -c -g -Wall -Wextra -std=c++11
TARGETS=FenJiA FenJiABase
LIBs=$(addsuffix .so,$(addprefix ./lib/lib, $(TARGETS)))
LIB=-lm -L/usr/local/lib -lboost_date_time -lboost_filesystem -lboost_system -lboost_timer
INCLUDE=-I./includes -I/usr/local/include
BIN=Main Test
RST=$(wildcard, doc/*.rst)

all:$(addprefix bin/, $(BIN))

lib/lib%.so:cpp/%.cpp 
	$(CXX) $(CXX_FLAGS) -o $@ $< $(LIB) $(INCLUDE) 
bin/Main:cpp/Main.cpp $(LIBs)
	$(CXX) cpp/Main.cpp $(LIBs) $(LIB) -o $@ -std=c++11 $(INCLUDE)
bin/Test:cpp/Test.cpp $(LIBs)
	$(CXX) cpp/Test.cpp $(LIBs) $(LIB) -o $@ -std=c++11 $(INCLUDE)
doc:$(INCLUDE) $(RST)
	doxygen Doxyfile
	make -C doc html
clean:
	rm -rf bin/*
	rm -rf lib/*
