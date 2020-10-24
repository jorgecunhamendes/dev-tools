#
# Make dev-tools
#
# Available tools to be built:
#
#   commands - execute and show the output of the commands given via stdin
#
# Other targets:
#
#   clean    - remove intermediate build artefacts
#   cleanall - remove all build artefacts, including binaries

.PHONY : commands clean cleanall

commands : bin/commands

clean :
	@echo rm -r build
	@if [ -d build ] ; then rm -r build ; fi

cleanall : clean
	@echo rm -r bin
	@if [ -d bin ] ; then rm -r bin ; fi

#
# tool: commands
#

COMMANDS_DEPS=\
	build/commands/command.o\
	build/commands/menu.o \
	build/commands/main.o

build/commands/command.o : commands/command.c commands/command.h
build/commands/menu.o : commands/menu.c commands/menu.h
build/commands/main.o : commands/main.c commands/command.h commands/menu.h

LIBS=glib-2.0 gtk+-3.0 vte-2.91

CFLAGS=-O2 -Wall `pkg-config --cflags $(LIBS)`
LDFLAGS=`pkg-config --libs $(LIBS)`

bin/commands : $(COMMANDS_DEPS) | bin
	gcc -o $@ $^ $(LDFLAGS)

build/commands/%.o : commands/%.c | build/commands
	gcc -c $(CFLAGS) -o $@ $<

#
# directory structure
#

DIRS=\
	bin \
	build \
	build/commands \


$(DIRS) :
	mkdir -p $@
