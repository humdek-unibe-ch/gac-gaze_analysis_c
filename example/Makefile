SHELL := /bin/bash
APPNAME = example
SOURCES = example.c

INCLUDES_DIR = -I../cglm/include \
			   -I../include

LIBDIR = ../build/lib
LD_LIBRARY_PATH = LD_LIBRARY_PATH=$(LIBDIR)

LINK_DIR = -L$(LIBDIR)

LINK_FILE = -lm \
			-lgac

CFLAGS = -Wall
DEBUG_FLAGS = -g -O0

CC = gcc

all: $(APPNAME)

# compile with dot stuff and debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

run: $(APPNAME)
	$(LD_LIBRARY_PATH) ./$(APPNAME)

$(APPNAME): $(SOURCES) $(LIBDIR)/libgac.so
	$(CC) $(CFLAGS) $(SOURCES) $(INCLUDES_DIR) $(LINK_DIR) $(LINK_FILE) -o $@

$(LIBDIR)/libgac.so:
	$(MAKE) -C ../..

.PHONY: clean

clean:
	rm -f $(APPNAME)

valgrind: 
	$(LD_LIBRARY_PATH) valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./$(APPNAME)
