# Makefile for pattern
#

CC = gcc
CFLAGS = -Wall -O2

pattern:
	${CC} ${CFLAGS} -o pattern pattern.c

clean:
	rm -rf pattern *~

