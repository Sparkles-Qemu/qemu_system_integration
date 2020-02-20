all: debug

debug:
	g++ -g -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 SIM.cpp -o sim
release:
	g++ -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 SIM.cpp -o sim
clean:
	rm -rf ./sim
run:
	./sim;