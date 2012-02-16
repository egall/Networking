#Erik Steggall
#Assignment 1
#CMPS 156
#01/25/12

SOURCES = Makefile README client.c server.c
PROJECT = cmpe156 asg 1

server : server.o client.o
	gcc -o server server.o

server.o : server.c
	gcc -c server.c

#client : client.o
#	gcc -o client client.o

client.o : client.c
	gcc -o client client.c

clean : 
	rm -f *.o server client
