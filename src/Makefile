WORKING_DIR:=$(shell pwd)

all: debug

debug:
	g++ -g -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 mux.cpp tb.cpp main.cpp -o mux
release:
	g++ -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 mux.cpp tb.cpp main.cpp -o mux
clean:
	rm -rf ./mux
run:
	./mux
	
gtkwave:
	ssh gateway -X gtkwave $(WORKING_DIR)/$(WAVE)