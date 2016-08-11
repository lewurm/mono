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

monodir=$(top_builddir)

if ENABLE_INTERPRETER
bin_PROGRAMS = mint

lib_LTLIBRARIES = libmint.la

man_MANS = mint.1
endif

mint_SOURCES =		\
	main.c

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

#  $(monodir)/mono/mini/libmonosgen-2.0.la

# TODO: CoreFoundation wtf, there must be a better way.

mint_LDFLAGS = -Wl,-framework -Wl,CoreFoundation

mint_LDADD = \
	libmint.la \
	$(monodir)/mono/metadata/libmonoruntimesgen-static.la \
	$(monodir)/mono/sgen/libmonosgen-static.la \
	$(monodir)/mono/io-layer/libwapi.la	\
	$(monodir)/mono/utils/libmonoutils.la \
	$(GLIB_LIBS) \
	$(LIBICONV) \
	$(ICU_LIBS) \
	-lm

EXTRA_DIST = $(man_MANS)
