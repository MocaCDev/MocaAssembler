.PHONY: build
.PHONY: run
.PHONY: clean

CC=g++
FLAGS = -std=c++20 -fsanitize=leak -o

build: bin/main.o
run: build
	./bin/main.o $(asm)

clean_main:
	rm -rf bin/main.o

bin/main.o: clean_main
	$(CC) main.cpp $(FLAGS) bin/main.o

clean:
	rm -rf bin/*