######################################################################
#
# 	Makefile for Simple alpha PROXY Server / wcol
#		ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
#		UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
#
#	$Id$
#
######################################################################
# Edit follows, for suit your systems.

# .SUFFIXES: $(SUFFIXES) 

#
# Compile Flag
#
#	DEBUG			- debug mode.
#	USE_SYSLOG		- use syslog() for logging.
#	SLOT_FORK_PREFETCH 	- use fork() for pre-fetch by hash-slot.
#	ALL_FORK_PREFETCH 	- use fork() for pre-fetch by each data.
#

#DEFS = -DDEBUG -DALL_FORK_PREFETCH
#DEFS = -DDEBUG -DUSE_SYSLOG -DSLOT_FORK_PREFETCH
#DEFS = -DDEBUG
#DEFS = -DSLOT_FORK_PREFETCH

#
# Commands and Options
#

# --- 	usual compiletion
#CFLAGS = -g $(DEFS)

# ---	if you wanna run gcc
CC = gcc
CFLAGS = -pipe -ansi $(DEFS)

# add -g to "Produce debugging information in the operating system's native format"
# add -Wall if you are bored and are looking for some things to work on... ;-)

# ---   you must use GNU tar, if you want pack this sources.
TAR	  = tar
RM 	  = rm -f


# --- 	if you use Sun workstation
#LIBS = -lnsl -lsocket -lucb
#LDFLAGS= -Bstatic



######################################################################
# Don't touch follows...

#
# Files
#

AIM    = wcol
HEADS  = wcol.h conv.h
OBJS   = base.o msg.o tcp.o accept.o get.o http.o send.o conv.o
MOBJ   = main.o

MISCS  = README NOTE CHANGES TODO Makefile wcol-accept.cfg wcol-conv.cfg
SRCS   = ${OBJS:.o=.c} ${MOBJ:.o=.c}


#
# targets
#

all: $(AIM)

wcol: $(OBJS) main.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o wcol main.o $(OBJS) $(LIBS)

pack:
	$(TAR) zcvf ../pack.`date +%y%m%d_%H%M%S`.tar.gz  $(HEADS) $(SRCS) $(MISCS)

smp: smp.c base.o msg.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o a.out smp.c base.o msg.o

clean:
	$(RM) $(AIM) $(MOBJ) $(OBJS)

#
# dependency
#
$(OBJS): wcol.h
send.o: conv.h
conv.o: conv.h
# pa.o: pa.h

# --- End of Makefile ---
