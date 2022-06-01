loopback: 
	gcc loopback.c src/mesafpga.c -I../mesaflash -Iinclude/ -L../mesaflash -lm -lsocket -lanyio -o loopback
 
all: loopback

clean:
	rm -rf loopback *.o
