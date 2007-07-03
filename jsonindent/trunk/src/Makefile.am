bin_PROGRAMS = jsonindent
jsonindent_SOURCES = json.c jsonindent.c

# set the include path found by configure
INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/rstring $(all_includes)

# the library search path.
jsonindent_LDFLAGS = $(all_libraries) 
SUBDIRS = rstring
jsonindent_LDADD = $(top_builddir)/src/rstring/librstring.a
