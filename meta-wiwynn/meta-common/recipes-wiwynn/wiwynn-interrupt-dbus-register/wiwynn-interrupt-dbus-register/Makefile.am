AM_DEFAULT_SOURCE_EXT = .cpp

nobase_include_HEADERS = \
	conf.hpp \
	exception.hpp

libjsonparserdir = ${libdir}
libjsonparser_LTLIBRARIES  = libjsonparser.la

libjsonparser_la_LDFLAGS = -shared

libjsonparser_la_SOURCES = jsonparser.cpp

bin_PROGRAMS = interruptdbusregister

interruptdbusregister_SOURCES = main.cpp

interruptdbusregister_LDADD = \
	$(SDBUSPLUS_LIBS) \
        libjsonparser.la \
	-lgpiod

interruptdbusregister_CXXFLAGS = \
	$(SDBUSPLUS_CFLAGS)


