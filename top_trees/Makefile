PROG=top_trees_test
CLASSES=BaseTree TopTree
OTHER=
DIRECTORIES=obj

OBJS=$(addprefix obj/,${OTHER} $(addsuffix .o,${CLASSES} ${PROG}))

INC=-Isrc -Iinclude

CFLAGS=-Wall -std=c++14 -c
LDFLAGS=-Wall
CC=g++

all: directories ${PROG}

obj/%.o: src/%.cpp
	${CC} ${CFLAGS} ${INC} -o $@ $<

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} ${INC} -o $@ $^

clean:
	rm -f ${PROG} ${OBJS}

directories:
	mkdir -p ${DIRECTORIES}

.PHONY: clean all directories

.SECONDARY:
