CC=clang

CFLAGS=-std=c99 -Wall -pedantic

all: pa1 pa1.tar.gz

pa1.tar.gz: src/*.c src/*.h
	tar -cvzf pa1.tar.gz -C src .

pa1: src/*.c src/*.h
	${CC} ${CFLAGS} src/*.c -o pa1
	
