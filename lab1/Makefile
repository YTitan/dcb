CC=clang

CFLAGS=-std=c99 -Wall -pedantic

all: main pa1.tar.gz

pa1.tar.gz: src/*.c src/*.h
	mkdir pa1
	cp -r src/* pa1
	cd pa1
	tar -cvzf pa1.tar.gz pa1
	cd ..
	rm -r pa1

main: src/*.c src/*.h
	${CC} ${CFLAGS} src/*.c -o main 
	
