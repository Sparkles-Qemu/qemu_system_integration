CXX=g++
DEFS=-DPACKAGE_NAME=\"systemc\" -DPACKAGE_TARNAME=\"systemc\" -DPACKAGE_VERSION=\"2.3.0\" -DPACKAGE_STRING=\"systemc\ 2.3.0\" -DPACKAGE_BUGREPORT=\"\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\"
DEFAULT_INCLUDES = -I.

prefix=/Users/Student/$(USER)/Sparkles-QEMU/qemu_system_integration/src/systemc

INCLUDES=-I$(prefix)/include
CPPFLAGS= 
AM_CXXFLAGS=-Wall -DSC_INCLUDE_FX -O3
CXXFLAGS=-g -O2
LIBXX=-L/Users/Student/$(USER)/Sparkles-QEMU/qemu_system_integration/src/systemc/lib-linux64 -lsystemc -lm

#edit top_builddir: this var should point to systemc folder
top_builddir = ../systemc
LIBTOOL=$(SHELL) $(top_builddir)/libtool
AM_CXXFLAGS=-Wall -DSC_INCLUDE_FX -O3

#Edit all these vars to match your sources and targets
TARGET=
SRCS=
SUBDIRS= DMA MultiplyAndAccumulate
OBJS=
SUBDIR_OBJS=
ALL_OBJS= $(OBJS) $(SUBDIR_OBJS) 

CXXCOMPILE=$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CXXFLAGS) $(CXXFLAGS)
			 			 	--mode=compile $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
																				$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLD=$(CXX)

CXXLINK=$(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) \
																--mode=link $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) \
																									$(LDFLAGS)

#If you have more than .o file make sure to added below and in the OBJS var
all: $(TARGET)

$(TARGET): $(ALL_OBJS)
		$(CXXLINK) -o $@ $^ $(LIBXX)

subdir_compile:
		for dir in $(SUBDIRS); do \
			$(MAKE) -C $$dir; \
		done
		
run_tests:

clean_all:
		for dir in $(SUBDIRS); do \
			$(MAKE) clean  -C $$dir; \
		done

clean: 
		rm $(TARGET) $(OBJS)
