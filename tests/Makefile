# Makefile

COMPILER?=clang
OPT?=3
CC=${COMPILER}
FLAGS?=-Wall -O${OPT} -I.. -DVERBOSE
LIBS?=
TARGETS=${subst .c,, ${shell find . -type f -name '*.c'}}

include ../platform.mk

all: ${TARGETS}

%: %.c
	${CC} $< -o $@ ${FLAGS} ${LIBS}

clean:
	@for prog in ${TARGETS}; do \
		rm $$prog; \
	done

run:
	@for prog in ${TARGETS}; do \
		echo $$prog; \
		$$prog; \
	done

.PHONY: clean
