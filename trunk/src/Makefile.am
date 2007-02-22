bin_PROGRAMS = json

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
json_LDFLAGS = $(all_libraries) 
noinst_HEADERS = helper.h json.h
json_SOURCES = helper.c json.c test.c
json_LDADD = $(top_builddir)/src/mstring/libmstring.a
SUBDIRS = rstring
