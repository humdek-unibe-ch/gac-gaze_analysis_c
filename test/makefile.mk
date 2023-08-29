# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

SHELL := /bin/bash
APPNAME = test
SOURCES = test.c

INCLUDES_DIR = -I../minunit \
			   -I../../cglm/include \
			   -I../../include

LIBDIR = ../../.libs
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
