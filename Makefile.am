# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

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