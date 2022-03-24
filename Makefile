CXXFLAGS = -Wall -MMD

#
# Project files
#
SRCS = graph.cc gain_container.cc FMpart.cc
OBJS = $(SRCS:.cc=.o)
EXE  = FMpart

.PHONY: all clean release debug prep

all: debug

prep:
	mkdir release debug

clean:
	rm -rf release/
	rm -rf debug/
	rm -f *.d
	rm -f *.gcov *.gcda *.gcno
	rm -f *.dot

release: CXXFLAGS += -O3 -DNDEBUG
release: release/FMpart

release/$(EXE): $(addprefix release/, $(OBJS))
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@
	cp release/$(EXE) ./$(EXE)

release/%.o: %.cc
	$(COMPILE.cc) -o $@ $<	

debug: CXXFLAGS += -g
debug: debug/FMpart

debug/$(EXE): $(addprefix debug/, $(OBJS))
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@
	cp debug/$(EXE) ./$(EXE)

debug/%.o: %.cc
	$(COMPILE.cc) -o $@ $<	

-include *.d
