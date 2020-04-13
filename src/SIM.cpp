#include "systemc.h"
#include "DMA.cpp"

using std::cout;
using std::endl;

#define RAM_SIZE 20

void print_ram(float* ram, int size)
{
	for (int i = 0; i < size; i++)
		std::cout << *(ram + i) << " ";
	std::cout << "\n\n";
}

/**
 * @brief Simulation entry point note sc_main not main.... 
 * a systemc idiosyncrasy related to the linking behavior. 
 * 
 * @param argc number of arguments passed (just like regular C )
 * @param argv arguments passed (just like regular C )
 * @return int 
 */
int sc_main(int argc, char *argv[]) 
{
	sc_signal<bool> clk("clk"); 
	sc_signal<bool> reset("reset"); 
	sc_signal<bool> enable("enable"); 
	
	// RAM representing external Memory
	float ram_s[RAM_SIZE], ram_d[RAM_SIZE] = {0};

	// Fill source ram with 1-100
	int i;
	for (i = 0; i < RAM_SIZE; i++)
		ram_s[i] = i + 1;

	// Instantiate MM2MM DMA
	DMA_MM2MM dma("dma_mm2mm", clk, reset, enable, ram_s, ram_d);
	Descriptor desc_mm2s = {0, 0, DmaState::SUSPENDED, RAM_SIZE, 1};
	Descriptor desc_s2mm_wait = {1, 0, DmaState::SUSPENDED, 1, 1};
	Descriptor desc_s2mm = {0, 0, DmaState::TRANSFER, RAM_SIZE, 1};
	dma.mm2s.descriptors.push_back(desc_mm2s);
	dma.s2mm.descriptors.push_back(desc_s2mm_wait);
	dma.s2mm.descriptors.push_back(desc_s2mm);

	std::cout << "\nsource ram: " << std::endl;
	print_ram(ram_s, RAM_SIZE);
	std::cout << "destination ram: " << std::endl;
	print_ram(ram_d, RAM_SIZE);

	clk.write(0);
	reset.write(0);
	enable.write(1);

	// reset to initialize
	reset.write(1);
	sc_start(1, SC_NS);
	reset.write(0);

	dma.mm2s.descriptors[0].state = DmaState::TRANSFER;
	dma.s2mm.descriptors[0].state = DmaState::WAIT;

	// start transfer of data
	for (i = 0; i < RAM_SIZE + 1; i++)
	{
		clk.write(1);
		sc_start(1, SC_NS);
		clk.write(0);
		sc_start(1, SC_NS);
	}

	std::cout << "\nsource ram: " << std::endl;
	print_ram(ram_s, RAM_SIZE);
	std::cout << "destination ram: " << std::endl;
	print_ram(ram_d, RAM_SIZE);

	return 0;
}
