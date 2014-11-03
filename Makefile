# Compiler
CC=g++
# Compiler Flags
FLAGS=-Wall -Werror -ansi -pedantic

all: rshell cp

rshell: 
		mkdir -p bin
		$(CC) $(FLAGS) src/main.cpp -o bin/rshell

cp:
		$(CC) $(FLAGS) src/cp.cpp -o bin/cp
