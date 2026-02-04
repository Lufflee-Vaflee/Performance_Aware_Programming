CXXFLAGS = -std=c++23 -Wall -pedantic -fstrict-aliasing -g -O1

ASM_FILES := $(wildcard test/*.asm)
TEST_SCRIPT := ./test/test.sh
INC=-I./include/

.PHONY: all build clean test dir $(ASM_FILES)

all: build test

build: build/8086sim

build/8086sim: build/main.o build/decode.o build/lex.o build/state_transition.o
	g++ -o build/8086sim build/main.o build/decode.o build/lex.o build/state_transition.o

build/main.o: src/main.cpp include/decode.hpp include/lex.hpp
	g++ -c $(CXXFLAGS) $(INC) src/main.cpp -o build/main.o

build/decode.o: src/decode.cpp include/decode.hpp include/op.hpp
	g++ -c $(CXXFLAGS) $(INC) src/decode.cpp -o build/decode.o

build/lex.o: src/lex.cpp include/lex.hpp include/op.hpp
	g++ -c $(CXXFLAGS) $(INC) src/lex.cpp -o build/lex.o

build/state_transition.o: src/state_transition.cpp include/state_transition.hpp include/op.hpp
	g++ -c $(CXXFLAGS) $(INC) src/state_transition.cpp -o build/state_transition.o

include/code.hpp: include/bit3mask.hpp

include/LT.hpp: include/code.hpp

include/lex.hpp: include/decode.hpp

include/op.hpp: include/LT.hpp

test: build
	./test/test.sh ./build/8086sim ./test

clean:
	rm -rf ./build/*
	rm -f ./test/*.indirect
	rm -f ./test/*.decoded*
	rm -f ./test/*.direct
	rm -f ./test/*.bin

