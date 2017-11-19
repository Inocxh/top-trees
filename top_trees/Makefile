PROG=top_trees_test
CLASSES=BaseTree TopTree Cluster
OTHER=
DIRECTORIES=obj

OBJS=$(addprefix obj/,${OTHER} $(addsuffix .o,${CLASSES} ${PROG}))
DEPS=$(OBJS:.o=.d)

INC=-Isrc -Iinclude

CFLAGS=-Wall -g -std=c++14 -c
LDFLAGS=-Wall
CC=g++

all: directories ${PROG}

# Dependencies:
-include $(DEPS)

obj/%.o: src/%.cpp
	${CC} ${CFLAGS} ${INC} -MMD -o $@ $<

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} ${INC} -o $@ $^

directories: ${DIRECTORIES}

${DIRECTORIES}:
	mkdir -p ${DIRECTORIES}

# Visualize dot

%.pdf: %.dot
	dot -Tps2 $< -o $*.ps
	ps2ps $*.ps $*-fixed.ps
	ps2pdf $*-fixed.ps $@
	rm -f $**.ps

clean:
	rm -f ${PROG} ${OBJS} ${DEPS}

.PHONY: clean all depend directories

.SECONDARY:
