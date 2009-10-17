# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = gnu

SUBDIRS = src

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=mjson-@MILESTONE@.pc

mjson-@MILESTONE@.pc: mjson.pc Makefile
	cmp -s $< $@ || cp $< $@

CLEANFILES=$(pkgconfig_DATA)

