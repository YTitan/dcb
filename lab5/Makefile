CC=clang

CFLAGS=-std=c99 -Wall -pedantic 

NUM=5

all: main tarball

tarball: src/*.c src/*.h
	mkdir pa${NUM}
	cp -r src/* pa${NUM}
	cd pa${NUM}
	tar -cvzf pa${NUM}.tar.gz pa${NUM}
	cd ..
	rm -r pa${NUM}

main: src/*.c src/*.h
	${CC} ${CFLAGS} src/*.c -o main -L./src/lib64 -lruntime 
	
