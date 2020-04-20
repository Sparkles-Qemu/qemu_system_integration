#include <systemc.h>
#include "LEFT.cpp"


int sc_main(int argc, char *argv[])
{
	sc_signal<bool> clk("clk"); 
	sc_signal<bool> reset("reset"); 
	sc_signal<bool> enable("enable"); 
	sc_signal<float, SC_MANY_WRITERS> stream("stream");	// SC_MANY_WRITERS allows stream to have numerous drivers

	clk.write(0);
	reset.write(0);
	enable.write(0);

	// RAM representing external Memory
	float ram[100], ram1[100] = {0}, ram2[100] = {0}, ram3[100] = {0};

	// Fill ram with 1-100
	int i;
	for (i = 0; i < 100; i++)
		ram[i] = i;

	// instantiate left side of processor
	LEFT left("left_side", clk, reset, enable, ram, ram1, ram2, ram3);

	return 0;

}
