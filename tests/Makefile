# Makefile

COMPILER?=gcc
OPT?=0
CC=${COMPILER}
FLAGS?=-Wall -O${OPT} -I.. -DVERBOSE
LIBS?=
TARGETS=${subst .c,, ${shell find . -type f -name '*.c'}}

include ../platform.mk

all: ${TARGETS}

%: %.c
	${CC} $< -o $@ ${FLAGS} ${LIBS}

clean:
	rm ${TARGETS}

run:
	@for prog in ${TARGETS}; do \
		echo $$prog; \
		$$prog; \
	done

.PHONY: clean
