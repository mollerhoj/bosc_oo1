all: bosh

OBJS = bosh.o parser.o print.o
LINKPATH=-L/lib/i386-linux-gnu/
LIBS= -l readline -ltermcap
CC = gcc

bosh: bosh.o ${OBJS}
	${CC} -o $@ ${OBJS} ${LINKPATH} ${LIBS} 

clean:
	rm -rf *o bosh
