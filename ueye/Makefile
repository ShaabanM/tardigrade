all: test_ueye

test_ueye: main.c camera.c camera.h
	gcc -Wall -g -o test_ueye camera.c main.c

clean:
	rm -f test_ueye *.o
