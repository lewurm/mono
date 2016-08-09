AM_CPPFLAGS = 				\
	-I$(top_srcdir)			\
	$(LIBGC_CPPFLAGS)		\
	$(GLIB_CFLAGS)

if HOST_WIN32
export HOST_CC
# The mingw math.h has "extern inline" functions that dont appear in libs, so
# optimisation is required to actually inline them
AM_CFLAGS = -O
endif

if ENABLE_INTERPRETER
bin_PROGRAMS = mint

lib_LTLIBRARIES = libmint.la

man_MANS = mint.1
endif

mint_SOURCES =		\
	main.c

mint_LDADD =		\
	libmint.la

libmint_la_SOURCES =	\
	hacks.h		\
	interp.h	\
	interp.c	\
	mintops.h	\
	mintops.def	\
	mintops.c	\
	transform.c

libmintincludedir = $(includedir)/mono-$(API_VER)/mono/interpreter

libmintinclude_HEADERS = 	\
	embed.h

libmint_la_LIBADD =				\
	../arch/libmonoarch.la		\
	../metadata/libmonoruntime.la	\
	../io-layer/libwapi.la		\
	../utils/libmonoutils.la	\
	$(GLIB_LIBS)			\
	$(LIBICONV)			\
	$(ICU_LIBS)			\
	-lm

EXTRA_DIST = $(man_MANS)
