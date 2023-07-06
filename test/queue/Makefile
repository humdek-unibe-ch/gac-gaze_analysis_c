SHELL := /bin/bash
APPNAME = test
SOURCES = test.c

INCLUDES_DIR = -I../minunit \
			   -I../../cglm/include \
			   -I../../include

LIBDIR = ../../build/lib
LD_LIBRARY_PATH = LD_LIBRARY_PATH=$(LIBDIR)

LINK_DIR = -L$(LIBDIR)

LINK_FILE = -lrt \
			-lm \
			-lgac

CFLAGS = -Wall
DEBUG_FLAGS = -g -O0

CC = gcc

all: $(APPNAME)

# compile with dot stuff and debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

$(APPNAME): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(INCLUDES_DIR) $(LINK_DIR) $(LINK_FILE) -o $@

.PHONY: clean

clean:
	rm -f $(APPNAME)

run:
	$(LD_LIBRARY_PATH) ./$(APPNAME)

valgrind: 
	$(LD_LIBRARY_PATH) valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./$(APPNAME)
