CC ?= gcc

DBGCFLAGS := -g -O0 -DDEBUG
RELCFLAGS := -O3 -DNDEBUG
CFLAGS = -std=c2x -Wall -fmax-errors=10 -Wextra -I$(IDIR)
PKGCONFIG ?= $(shell which pkg-config)
SDLCONFIG ?=
MYSQLCONFIG ?= $(shell which mysql_config)
LFLAGS := $(shell $(PKGCONFIG) gtk4 --cflags) $(shell $(MYSQLCONFIG) --cflags)
LIBS := $(shell $(PKGCONFIG) gtk4 --libs) $(shell $(MYSQLCONFIG) --libs)

# Project directories
IDIR := include
SRCDIR := src
BINDIR := bin
DBGDIR := debug
RELDIR := release

# Header files
_DEPS := %.h
DEPS := $(patsubst %, $(IDIR)/%, $(_DEPS))

# Object files
_OBJS := main.o window.o dataquery.o
RELOBJS := $(patsubst %,$(RELDIR)/%,$(_OBJS))
DBGOBJS := $(patsubst %,$(DBGDIR)/%,$(_OBJS))

# Executable files
_BIN := exefile
DBGEXE := $(patsubst %,$(DBGDIR)/$(BINDIR)/%,$(_BIN))
RELEXE := $(patsubst %,$(RELDIR)/$(BINDIR)/%,$(_BIN))

# Default build
all: prep $(RELEXE) $(DBGEXE)

# Release build rule
$(RELEXE): $(RELOBJS)
	$(CC) $(CFLAGS) $(RELCFLAGS) $(LIBS) -o $@ $^

$(RELDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(RELCFLAGS) $(LFLAGS) -c -o $@ $<

# Debug build rule
$(DBGEXE): $(DBGOBJS)
	$(CC) $(CFLAGS) $(DBGCFLAGS) $(LIBS) -o $@ $^

$(DBGDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(DBGCFLAGS) $(LFLAGS) -c -o $@ $<

# Create directories if not exist
prep:
	@mkdir -p $(DBGDIR)/$(BINDIR) $(RELDIR)/$(BINDIR)

# Run exe
run:
	./$(RELEXE)

rund:
	./$(DBGEXE)

.PHONY: clean init

clean:
	rm -f $(RELEXE) $(RELOBJS) $(DBGEXE) $(DBGOBJS)

# Create sample project
init:
	mkdir src include
	echo -e "int main() {" >> ./src/main.c
	echo -e "\treturn 0;" >> ./src/main.c
	echo -e "}" >> ./src/main.c
