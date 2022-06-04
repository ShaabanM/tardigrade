all: test_ueye

test_ueye: main.c camera.c camera.h
	gcc -Wall -g -o test_ueye -L/opt/ids/ueye/lib/ -I/opt/ids/ueye/include/ camera.c main.c -lueye_api

clean:
	rm -f test_ueye *.o
