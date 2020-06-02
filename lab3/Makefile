CC=clang

CFLAGS=-std=c99 -Wall -pedantic 

all: main pa3.tar.gz

pa3.tar.gz: src/*.c src/*.h
	mkdir pa3
	cp -r src/* pa3
	cd pa3
	tar -cvzf pa3.tar.gz pa3
	cd ..
	rm -r pa3

main: src/*.c src/*.h
	${CC} ${CFLAGS} src/*.c -o main -L./src/lib64 -lruntime 
	
