# Compiler
CC=g++
# Compiler Flags
FLAGS=-Wall -Werror -ansi -pedantic

all: rshell ls cp

rshell: 
		mkdir -p bin
		$(CC) $(FLAGS) src/main.cpp -o bin/rshell

ls:
		$(CC) $(FLAGS) src/ls.cpp -o bin/ls

cp:
		$(CC) $(FLAGS) src/cp.cpp -o bin/cp
