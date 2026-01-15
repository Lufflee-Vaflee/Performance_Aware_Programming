CXXFLAGS = -std=c++23 -Wall -pedantic -fstrict-aliasing -g

ASM_FILES := $(wildcard test/*.asm)
TEST_SCRIPT := ./test/test.sh

.PHONY: all build clean test dir $(ASM_FILES)

all: build test

build: dir build/8086sim

dir:
	mkdir -p build

build/8086sim: build/main.o
	g++ -o build/8086sim build/main.o

build/main.o: src/main.cpp
	g++ -c $(CXXFLAGS) src/main.cpp -o build/main.o

test: build
	./test/test.sh ./build/8086sim ./test

clean:
	rm -rf build
	rm -f ./test/*.bin

