CXXFLAGS = -std=c++23 -Wall -pedantic -fstrict-aliasing -g

ASM_FILES := $(wildcard test/*.asm)
TEST_SCRIPT := ./test/test.sh
INC=-I./include/

.PHONY: all build clean test dir $(ASM_FILES)

all: build test

build: build/8086sim

build/8086sim: build/main.o build/decode.o build/mov.o
	g++ -o build/8086sim build/main.o build/decode.o build/mov.o

build/main.o: src/main.cpp include/decode.hpp
	g++ -c $(CXXFLAGS) $(INC) src/main.cpp -o build/main.o

build/decode.o: src/decode.cpp include/LT.hpp include/mov.hpp include/opcode.hpp include/bit3mask.hpp
	g++ -c $(CXXFLAGS) $(INC) src/decode.cpp -o build/decode.o

build/mov.o: src/mov.cpp include/decode.hpp
	g++ -c $(CXXFLAGS) $(INC) src/mov.cpp -o build/mov.o


test: build
	./test/test.sh ./build/8086sim ./test

clean:
	rm -rf ./build/*
	rm -f ./test/*.indirect
	rm -f ./test/*.decoded*
	rm -f ./test/*.direct
	rm -f ./test/*.bin

