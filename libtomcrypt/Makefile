CC = gcc

#Release
CFLAGS = -O2 -Wall -I ..
#Debug
#CFLAGS = -g -Wall -I ..


OBJS = sha1.o sha256.o sha384.o sha512.o md5.o rmd160.o

all:  proof_of_work example

proof_of_work: proof_of_work.c  $(OBJS)
	($(CC) $(CFLAGS) -o proof_of_work proof_of_work.c sha1.o; mv proof_of_work ../cgi-bin/proof_of_work)

example: example.c  $(OBJS)
	$(CC) $(CFLAGS) -o example example.c sha1.o

clean:
	rm -f example *~ *.o
