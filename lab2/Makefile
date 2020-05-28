CC=clang

CFLAGS=-std=c99 -Wall -pedantic 

all: main pa2.tar.gz

pa2.tar.gz: src/*.c src/*.h
	mkdir pa2
	cp -r src/* pa2
	cd pa2
	tar -cvzf pa1.tar.gz pa2
	cd ..
	rm -r pa2

main: src/*.c src/*.h
	${CC} ${CFLAGS} src/*.c -o main -L./src/lib64 -lruntime 
	
