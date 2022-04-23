CC = gcc

#Release Flags
CFLAGS = -O2 -Wall -I .

#DebugFlags
#CFLAGS = -g -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: servidor adder cliente hashexample

server: servidor.c csapp.o
    $(CC) $(CFLAGS) -o servidor servidor.c csapp.o $(LIB)

csapp.o:
    $(CC) $(CFLAGS) -c csapp.c

adder:
    (cd cgi-bin; make)

hashexample:
    (cd libtomcrypt ; make)
