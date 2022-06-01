CC = gcc 
INCLUDE =  -I../mesaflash -Iinclude/ -I/usr/include  -I../mesafpga/include
LIBS = -L/home/bit/mesaflash -lanyio -lpci -lm 

loopback: 
	${CC} loopback.c src/mesafpga.c  ${INCLUDE} ${LIBS} -o loopback
 

all: loopback

clean:
	rm -rf loopback *.o
