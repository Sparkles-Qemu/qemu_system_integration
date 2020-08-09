
CXX=g++
DEFS=-DPACKAGE_NAME=\"systemc\" -DPACKAGE_TARNAME=\"systemc\" -DPACKAGE_VERSION=\"2.3.0\" -DPACKAGE_STRING=\"systemc\ 2.3.0\" -DPACKAGE_BUGREPORT=\"\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\"
DEFAULT_INCLUDES = -I.

prefix=/Users/Student/wgaldame/Sparkles-QEMU/qemu_system_integration/src/systemc

INCLUDES=-I$(prefix)/include
CPPFLAGS= 
AM_CXXFLAGS=-Wall -DSC_INCLUDE_FX -O3
CXXFLAGS=-g -O2

top_builddir = ../../..
LIBTOOL=$(SHELL) $(top_builddir)/libtool
AM_CXXFLAGS=-Wall -DSC_INCLUDE_FX -O3

TARGET=hello
SRCS=hello.cpp
OBJS=hello.o

CXXCOMPILE=$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
LTCXXCOMPILE=$(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) \
	--mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLD=$(CXX)
CXXLINK=$(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) \
	--mode=link $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) \
	$(LDFLAGS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXXLINK) -o $@ $^ -L/Users/Student/wgaldame/Sparkles-QEMU/qemu_system_integration/src/systemc/lib-linux64 -lsystemc -lm

%.o: hello.cpp
		$(CXXCOMPILE) -c -o $@ $^
