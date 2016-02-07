#Makefile

CC = gcc
CFLAGS = -O -Wall -pedantic -pthread

OBJS = main.o global_header.o clientslib.o servantslib.o genlib.o

myprogram: ${OBJS}
	${CC} -o myprogram ${CFLAGS} ${OBJS}


genlib.o: genlib.c genlib.h
	${CC} ${CFLAGS} -c genlib.c

servantslib.o: servantslib.c servantslib.h
	${CC} ${CFLAGS} -c servantslib.c

clientslib.o: clientslib.c clientslib.h
	${CC} ${CFLAGS} -c clientslib.c

global_header.o: global_header.c global_header.h
	${CC} ${CFLAGS} -c global_header.c

main.o: main.c
	${CC} ${CFLAGS} -c main.c

clean:
	rm -f myprogram ${OBJS}
	@echo "all cleaned up!"

