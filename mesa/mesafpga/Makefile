loopback: 
	gcc loopback.c src/mesafpga.c -I../mesaflash -Iinclude/ -L../mesaflash -lm -lpci -lanyio -o loopback
 
all: loopback

clean:
	rm -rf loopback *.o
