# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

include config.mk

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -I./include \
			-I./cglm/include \
			-DLIB_GAC_VERSION=\"$(VMAJ).$(VMIN).$(VREV)\"

lib_LTLIBRARIES = libgac.la

gacdir=$(includedir)
gac_HEADERS = include/gac.h \
			  include/gac_aoi.h \
			  include/gac_aoi_analysis.h \
			  include/gac_aoi_collection.h \
			  include/gac_aoi_collection_analysis.h \
			  include/gac_filter_fixation.h \
			  include/gac_filter_gap.h \
			  include/gac_filter_noise.h \
			  include/gac_filter_saccade.h \
			  include/gac_fixation.h \
			  include/gac_plane.h \
			  include/gac_queue.h \
			  include/gac_sample.h \
			  include/gac_saccade.h \
			  include/gac_screen.h

libgac_la_SOURCES = src/gac.c \
					src/gac_aoi.c \
					src/gac_aoi_analysis.c \
					src/gac_aoi_collection.c \
					src/gac_aoi_collection_analysis.c \
					src/gac_filter_fixation.c \
					src/gac_filter_gap.c \
					src/gac_filter_noise.c \
					src/gac_filter_saccade.c \
					src/gac_fixation.c \
					src/gac_plane.c \
					src/gac_queue.c \
					src/gac_sample.c \
					src/gac_saccade.c \
					src/gac_screen.c

libgac_la_LDFLAGS = -no-undefined -version-number $(VMAJ):$(VMIN):$(VREV)
