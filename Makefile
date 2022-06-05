CC = gcc

#Release Flags
#CFLAGS = -O2 -Wall -I .

#DebugFlags
CFLAGS = -g -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: servidor adder hashexample cliente 

servidor: servidor.c csapp.o
	$(CC) $(CFLAGS) -o servidor servidor.c csapp.o $(LIB)

csapp.o:
	$(CC) $(CFLAGS) -c csapp.c

adder:
	(cd cgi-bin; make)

hashexample:
	(cd libtomcrypt ; make)

cliente: cliente.c
	$(CC) $(CFLAGS) -o cliente cliente.c csapp.o ./libtomcrypt/sha1.o $(LIB)

clean:
	rm -f *.o servidor cliente *~
	(cd cgi-bin; make clean)
	(cd libtomcrypt ; make clean)