CC=gcc
CFLAGS=
NAME=memleak

all: memleak

memleak: memleak.c
	${CC} ${CFLAGS} -o ${NAME} $<

clean:
	rm -f ${NAME}
