#Erik Steggall
#Assignment 1
#CMPS 156
#01/25/12

SOURCES = Makefile README myclient.c myserver.c time_server.c
PROJECT = cmpe156 asg 1

myserver : myserver.o myclient.o
	gcc -o myserver myserver.o

myserver.o : myserver.c
	gcc -c myserver.c

#myclient : myclient.o
#	gcc -o myclient myclient.o

myclient.o : myclient.c
	gcc -o myclient myclient.c

clean : 
	rm -f *.o myserver myclient
