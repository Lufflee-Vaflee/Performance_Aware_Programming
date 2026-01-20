CXXFLAGS = -std=c++23 -Wall -pedantic -fstrict-aliasing -g -O1

ASM_FILES := $(wildcard test/*.asm)
TEST_SCRIPT := ./test/test.sh
INC=-I./include/

.PHONY: all build clean test dir $(ASM_FILES)

all: build test

build: build/8086sim

build/8086sim: build/main.o build/decode.o build/mov.o build/add.o build/cmp.o build/sub.o
	g++ -o build/8086sim build/main.o build/decode.o build/mov.o build/add.o build/sub.o build/cmp.o

build/main.o: src/main.cpp include/decode.hpp
	g++ -c $(CXXFLAGS) $(INC) src/main.cpp -o build/main.o

build/decode.o: src/decode.cpp include/decode.hpp include/LT.hpp include/JT.hpp include/J_.hpp
	g++ -c $(CXXFLAGS) $(INC) src/decode.cpp -o build/decode.o

build/mov.o: src/mov.cpp include/JT.hpp
	g++ -c $(CXXFLAGS) $(INC) src/mov.cpp -o build/mov.o

build/add.o: src/add.cpp include/JT.hpp
	g++ -c $(CXXFLAGS) $(INC) src/add.cpp -o build/add.o

build/sub.o: src/sub.cpp include/JT.hpp
	g++ -c $(CXXFLAGS) $(INC) src/sub.cpp -o build/sub.o

build/cmp.o: src/cmp.cpp include/JT.hpp
	g++ -c $(CXXFLAGS) $(INC) src/cmp.cpp -o build/cmp.o

include/LT.hpp: include/opcode.hpp

include/opcode.hpp: include/bit3mask.hpp

include/JT.hpp: include/decode.hpp

test: build
	./test/test.sh ./build/8086sim ./test

clean:
	rm -rf ./build/*
	rm -f ./test/*.indirect
	rm -f ./test/*.decoded*
	rm -f ./test/*.direct
	rm -f ./test/*.bin

