#ifndef PROCESSOR_CPP //be careful here, if there are duplicates used below they won't be used
#define PROCESSOR_CPP

#include <systemc.h>
#include <string>
#include "Right.cpp"
#include "LEFT.cpp"

#define IMAGE_WIDTH 10
#define IMAGE_HEIGHT 10
#define IMAGE_SIZE IMAGE_WIDTH *IMAGE_HEIGHT
#define SMALL_RAM_SIZE IMAGE_SIZE
#define BIG_RAM_SIZE 3 * IMAGE_SIZE
#define MEMORY_PADDING 100

struct Processor : public sc_module
{
	sc_signal<float> streamOut;

	float ramSource[BIG_RAM_SIZE + MEMORY_PADDING] = {0};
	float ram0[SMALL_RAM_SIZE + MEMORY_PADDING] = {0};
	float ram1[SMALL_RAM_SIZE + MEMORY_PADDING] = {0};
	float ram2[SMALL_RAM_SIZE + MEMORY_PADDING] = {0};

	LEFT left;
	RIGHT right;


	Processor(sc_core::sc_module_name name, const sc_signal<bool> &_clk, const sc_signal<bool> &_reset, const sc_signal<bool> &_enable) : sc_module(name),
																																		  left("mem_heirarchy", _clk, _reset, _enable, ramSource, ram0, ram1, ram2),
																																		  right("compute", _clk, _reset, _enable, ram0, ram1, ram2, streamOut)
	{
		std::cout << "Processor Module: " << name << " has been instantiated" << std::endl;
	}

	SC_HAS_PROCESS(RIGHT);
};

#endif // LEFT_CPP
