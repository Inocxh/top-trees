TESTER=top_trees_test
BINARIES=${TESTER} experiment_edge_weight experiment_double_edge_connectivity

TARGETS=${addprefix bin/,${BINARIES}}
CLASSES=BaseTree TopTree TopCluster TopologyCluster TopologyTopTree
OTHER=
DIRECTORIES=bin obj

OBJS=$(addprefix obj/,${OTHER} $(addsuffix .o,${CLASSES}))
DEPS=$(wildcard obj/*.d)

INC=-Isrc -Iinclude

CFLAGS=-Wall -g -std=c++14 -c
LDFLAGS=-Wall
CC=g++

all: directories ${TARGETS}

test: bin/${TESTER}
	./$< > test.dot
	make test.pdf

# Dependencies:
-include $(DEPS)

obj/%.o: src/%.cpp
	${CC} ${CFLAGS} ${INC} -MMD -o $@ $<

bin/%: obj/%.o ${OBJS}
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
	rm -f ${TARGETS} ${OBJS} ${DEPS}
	rm -r ${DIRECTORIES}

.PHONY: clean all depend directories test

.SECONDARY:
