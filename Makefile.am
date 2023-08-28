include config.mk

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -Wall \
            -O3 \
			-I./include \
			-I./cglm/include \
			-DLIB_GAC_VERSION=\"$(VMAJ).$(VMIN).$(VREV)\"

lib_LTLIBRARIES = libgac.la

gacdir=$(includedir)
gac_HEADERS = include/gac.h

libgac_la_SOURCES = src/gac.c
libgac_la_LDFLAGS = -no-undefined -version-number $(VMAJ):$(VMIN):$(VREV)
