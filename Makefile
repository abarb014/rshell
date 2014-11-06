# Compiler
CC=g++
# Compiler Flags
FLAGS=-Wall -Werror -ansi -pedantic

all: rshell ls

rshell: 
		mkdir -p bin
		$(CC) $(FLAGS) src/main.cpp -o bin/rshell

ls:
		$(CC) $(FLAGS) src/ls.cpp -o bin/ls
