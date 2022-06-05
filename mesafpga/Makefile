loopback: loopback.c 
	gcc loopback.c src/mesafpga.c -I../mesaflash -Iinclude/ -lanyio -lpci -lm -o loopback
 
all: loopback

clean:
	rm -rf loopback *.o
