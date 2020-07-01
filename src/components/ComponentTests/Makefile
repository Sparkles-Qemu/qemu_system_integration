CXX=g++ 

SYSC_PATH=../../systemc

INCLUDES=$(addprefix -I, $(shell find ../ -printf "%h/\n" | sort -u))
CXXFLAGS=-std=c++11 -g -Wall
LIBXX=-I$(SYSC_PATH)/include -L$(SYSC_PATH)/lib-linux64 -lsystemc -Wl,-rpath,$(SYSC_PATH)/lib-linux64  # need -Wl for dynamic linking

#Edit all these vars to match your sources and targets
TARGET= tb
OBJS= tb_generic.o

CXXCOMPILE=$(CXX) $(INCLUDES) $(CXXFLAGS) $(LIBXX)

all: clean $(TARGET)  

%.o: %.cpp 
	$(CXXCOMPILE) -c -o $@  $<

$(TARGET):  $(OBJS)
	$(CXXCOMPILE) $^ -o $@

RUN_TB: $(TARGET)
	./sim

clean: 
	rm -rf $(TARGET) $(OBJS) *.o
