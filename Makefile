# Makefile of ledctl_6501
#

CC=gcc -Wall -O2
CFLAGS=
#CFLAGS=-DDEBUG -DEXTDEBUG

LD=gcc
#LD=gcc -static

PROGRAMS = ledctl_6501

%.o:		%.cpp
		$(CC) $(CFLAGS) -c $< -o $@

all:		$(PROGRAMS)

ledctl_6501:	ledctl_6501.o
		$(LD) -o $@ $<

strip:		
		strip $(PROGRAMS)

clean:	
		rm -f *.o core $(PROGRAMS)
