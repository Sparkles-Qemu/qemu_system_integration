CXX=g++ 

SYSC_PATH=/usr/local/systemc-2.3.3

INCLUDES=$(addprefix -I, $(shell find ../ -printf "%h/\n" | sort -u))
CXXFLAGS=-std=c++11 -g -Wall
LIBXX=-I$(SYSC_PATH)/include -L$(SYSC_PATH)/lib-linux64 -lsystemc

#Edit all these vars to match your sources and targets
TARGET= tb
OBJS= Connector_tb.o

CXXCOMPILE=$(CXX) $(INCLUDES) $(CXXFLAGS) $(LIBXX)

all: clean $(TARGET)  

%.o: %.cpp 
	$(CXXCOMPILE) -c -o $@  $<

$(TARGET):  $(OBJS)
	$(CXXCOMPILE) $^ -o $@

RUN_TB: $(TARGET)
	./sim

clean_all: clean
	rm -rf  *.o *.vcd

clean: 
	rm -rf $(TARGET) $(OBJS)
