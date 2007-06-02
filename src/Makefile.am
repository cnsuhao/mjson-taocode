bin_PROGRAMS = json

# set the include path found by configure
INCLUDES = -I$(top_srcdir)/src/rstring $(all_includes)

# the library search path.
json_LDFLAGS = $(all_libraries) 
noinst_HEADERS = json.h saxy.h
json_SOURCES = json.c saxy.c test.c
json_LDADD = $(top_builddir)/src/rstring/librstring.a 
SUBDIRS = rstring
