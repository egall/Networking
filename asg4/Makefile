#Erik Steggall
#Assignment 4
#CMPS 156
#02/24/12

SOURCES = Makefile README client.c server.c
PROJECT = cmpe156 asg 1

server : server.o client.o
	gcc -o server server.o

server.o : server.c
	gcc -c server.c

client.o : client.c
	gcc -o client client.c

clean : 
	rm -f *.o server client
