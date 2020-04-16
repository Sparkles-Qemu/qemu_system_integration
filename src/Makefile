WORKING_DIR:=$(shell pwd)
SOURCE:=SIM.cpp
TARGET:=sim

all: debug

debug:
	g++ -std=c++11 -g -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 SIM.cpp DMA.cpp DMAtb.cpp -o $(TARGET)
release:
	g++ -std=c++11 -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 SIM.cpp DMA.cpp DMAtb.cpp -o $(TARGET)
clean:
	rm -rf ./$(TARGET)
run:
	./$(TARGET)
	
gtkwave:
	ssh gateway -X gtkwave $(WORKING_DIR)/$(WAVE) && echo "exit"
