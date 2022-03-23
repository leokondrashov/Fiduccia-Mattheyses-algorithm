CXXFLAGS ?= -Wall -MMD -g# --coverage

.PHONY: all clean

all: FMpart

clean:
	rm *.o
	rm *.d
	rm *.gcov *.gcda *.gcno
	rm *.dot
	rm main

test: graph.o gain_container.o

FMpart: graph.o gain_container.o

-include *.d
