thisdir = class/reference-assemblies

include ../../build/rules.make

all-local:

PROFILE_DIR=$(DESTDIR)$(mono_libdir)/mono/

install-local:
	$(MKINSTALLDIRS) $(PROFILE_DIR)/2.0
	$(MKINSTALLDIRS) $(PROFILE_DIR)/3.5
	$(MKINSTALLDIRS) $(PROFILE_DIR)/4.0

DISTFILES = Makefile

dist-local: dist-default
