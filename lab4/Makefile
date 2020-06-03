CC=clang

CFLAGS=-std=c99 -Wall -pedantic 

all: main pa4.tar.gz

pa4.tar.gz: src/*.c src/*.h
	mkdir pa4
	cp -r src/* pa4
	cd pa4
	tar -cvzf pa4.tar.gz pa4
	cd ..
	rm -r pa4

main: src/*.c src/*.h
	${CC} ${CFLAGS} src/*.c -o main -L./src/lib64 -lruntime 
	
