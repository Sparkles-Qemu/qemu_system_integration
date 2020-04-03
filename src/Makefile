WORKING_DIR:=$(shell pwd)

all: debug

debug:
	g++ -std=c++11 -g -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 SIM.cpp -o sim
release:
	g++ -std=c++11 -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 SIM.cpp -o sim
clean:
	rm -rf ./sim
run:
	./sim
	
gtkwave:
	ssh gateway -X gtkwave $(WORKING_DIR)/$(WAVE) && echo "exit"