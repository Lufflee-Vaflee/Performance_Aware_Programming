CXXFLAGS = -std=c++23 -O2 -Wall -pedantic -fstrict-aliasing

ASM_FILES := $(wildcard test/*.asm)
TEST_SCRIPT := ./test/test.sh

.PHONY: all build clean test dir $(ASM_FILES)

all: build test

build: dir build/8086sim

dir:
	mkdir -p build

build/8086sim: build/main.o
	g++ -o build/8086sim build/main.o

build/main.o:
	g++ -c $(CXXFLAGS) src/main.cpp -o build/main.o

test:
	./test/test.sh ./build/8086sim ./test/

clean:
	rm -rf build

