SHELL := /bin/bash

include config.mk

# Target OS detection
ifeq ($(OS),Windows_NT) # OS is a preexisting environment variable on Windows
	OS = windows
else
	UNAME := $(shell uname -s)
	ifeq ($(UNAME),Darwin)
		OS = macos
	else ifeq ($(UNAME),Linux)
		OS = linux
	else
    	$(error OS not supported by this Makefile)
	endif
endif

LLIBNAME = lib$(LIBNAME)
LOC_INC_DIR = include
LOC_SRC_DIR = src
LOC_BUILD_DIR = build
LOC_OBJ_DIR = $(LOC_BUILD_DIR)/obj
LOC_LIB_DIR = $(LOC_BUILD_DIR)/lib
CREATE_DIR = $(LOC_OBJ_DIR) $(LOC_LIB_DIR)

TEST_DIRS = $(wildcard ./test/test_*/.)
TESTS = $(wildcard ./test/test_*/test)

LIB_VERSION = $(VMAJ).$(VMIN)
UPSTREAM_VERSION = $(LIB_VERSION).$(VREV)
DEBIAN_REVISION = $(VDEB)
VERSION = $(UPSTREAM_VERSION)-$(DEBIAN_REVISION)

DYNLIB_EXT = so
# Windows-specific default settings
ifeq ($(OS),windows)
	DYNLIB_EXT = dll
endif

VLIBNAME = $(LLIBNAME)-$(LIB_VERSION)
SONAME = $(LLIBNAME).$(DYNLIB_EXT).$(LIB_VERSION)
ANAME = $(LLIBNAME).a

CGLM = cglm

STATLIB = $(LOC_LIB_DIR)/$(LLIBNAME).a
DYNLIB = $(LOC_LIB_DIR)/$(LLIBNAME).$(DYNLIB_EXT)

SOURCES = $(wildcard $(LOC_SRC_DIR)/*.c)
GAC_OBJECTS := $(patsubst $(LOC_SRC_DIR)/%.c, $(LOC_OBJ_DIR)/%.o, $(SOURCES))

INCLUDES = $(LOC_INC_DIR)/*.h

INCLUDES_DIR = -I$(CGLM)/include \
			   -I$(LOC_INC_DIR) \
			   -I.

LINK_DIR = -L/usr/local/lib

LINK_FILE = -lm

CFLAGS = -Wall -fPIC \
		 -DLIBSMXUTILS_VERSION_UP=\"$(UPSTREAM_VERSION)\"
DEBUG_FLAGS = -g -O0

CC = gcc

all: directories $(STATLIB) $(DYNLIB)

# compile with dot stuff and debug flags
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

$(STATLIB): $(GAC_OBJECTS)
	ar -cq $@ $^

$(DYNLIB): $(GAC_OBJECTS)
	$(CC) -shared -Wl,-soname,$(SONAME) $^ -o $@ $(LINK_DIR) $(LINK_FILE)
	ln -sf $(LLIBNAME).$(DYNLIB_EXT) $(LOC_LIB_DIR)/$(SONAME)

# compile project
$(LOC_OBJ_DIR)/%.o: $(LOC_SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES_DIR) -c $< -o $@ $(LINK_DIR) $(LINK_FILE)

directories: $(CREATE_DIR)

$(CREATE_DIR):
	mkdir -p $@

.PHONY: clean install uninstall doc cglm-check cglm-clean test $(TEST_DIRS)

install:

uninstall:

clean:
	rm -rf $(LOC_LIB_DIR)
	rm -rf $(LOC_OBJ_DIR)
	rm -rf $(LOC_BUILD_DIR)
	rm -f $(TESTS)

doc:
	doxygen .doxygen

cglm-check:
	$(MAKE) check -C $(CGLM)

cglm-clean:
	$(MAKE) clean -C $(CGLM)

test: $(TEST_DIRS)
$(TEST_DIRS):
	$(MAKE) run -C $@
