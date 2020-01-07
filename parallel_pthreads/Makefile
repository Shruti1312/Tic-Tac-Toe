all: main

main: main.o
	gcc -o main.exe main.o -lpthread

main.o: main.c
	gcc -o main.o -c main.c -lpthread