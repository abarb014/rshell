# Compiler
CC=g++
# Compiler Flags
FLAGS=-Wall -Werror -ansi -pedantic

all: rshell

rshell: 
		mkdir -p bin
		$(CC) $(FLAGS) src/main.cpp -o bin/rshell
