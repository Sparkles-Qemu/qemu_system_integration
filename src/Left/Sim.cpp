#include <systemc.h>
#include "DMA.cpp"
#include "LEFT.cpp"

#define SMALL_RAM_SIZE 10
#define BIG_RAM_SIZE 3 * SMALL_RAM_SIZE

void print_ram(float* ram, int size)
{
	for (int i = 0; i < size; i++)
		std::cout << *(ram + i) << " ";
	std::cout << "\n\n";
}

int sc_main(int argc, char *argv[])
{
	sc_signal<bool> clk("clk"); 
	sc_signal<bool> reset("reset"); 
	sc_signal<bool> enable("enable"); 
	sc_signal<float, SC_MANY_WRITERS> stream("stream");	// SC_MANY_WRITERS allows stream to have numerous drivers

	// RAM representing external Memory
	float ram[BIG_RAM_SIZE], ram1[SMALL_RAM_SIZE] = {0}, ram2[SMALL_RAM_SIZE] = {0}, ram3[SMALL_RAM_SIZE] = {0};

	// Fill source ram
	int i;
	for (i = 0; i < BIG_RAM_SIZE; i++)
		ram[i] = i;

	// instantiate left side of processor
	LEFT left("left_side", clk, reset, enable, ram, ram1, ram2, ram3);

	// descriptors for source ram
	Descriptor desc_mm2s = {0, 0, DmaState::TRANSFER, BIG_RAM_SIZE, 1};
	left.dma_mm2s.descriptors.push_back(desc_mm2s);

	// descriptors for destination ram 1
	Descriptor desc_s2mm1_wait_before = {1, 0, DmaState::WAIT, 1, 1};
	Descriptor desc_s2mm1_transfer = {2, 0, DmaState::TRANSFER, SMALL_RAM_SIZE, 1};	
	Descriptor desc_s2mm1_sus_after = {0, 0, DmaState::SUSPENDED, 0, 1};
	left.dma_s2mm1.descriptors.push_back(desc_s2mm1_wait_before);
	left.dma_s2mm1.descriptors.push_back(desc_s2mm1_transfer);
	left.dma_s2mm1.descriptors.push_back(desc_s2mm1_sus_after);

	// descriptors for destination ram 2
	Descriptor desc_s2mm2_wait_before = {1, 0, DmaState::WAIT, 1 + SMALL_RAM_SIZE, 1};
	Descriptor desc_s2mm2_transfer = {2, 0, DmaState::TRANSFER, SMALL_RAM_SIZE, 1};	
	Descriptor desc_s2mm2_sus_after = {0, 0, DmaState::SUSPENDED, 0, 1};
	left.dma_s2mm2.descriptors.push_back(desc_s2mm2_wait_before);
	left.dma_s2mm2.descriptors.push_back(desc_s2mm2_transfer);
	left.dma_s2mm2.descriptors.push_back(desc_s2mm2_sus_after);

	// descriptors for destination ram 3
	Descriptor desc_s2mm3_wait_before = {1, 0, DmaState::WAIT, 1 + 2 * SMALL_RAM_SIZE, 1};
	Descriptor desc_s2mm3_transfer = {2, 0, DmaState::TRANSFER, SMALL_RAM_SIZE, 1};	
	Descriptor desc_s2mm3_sus_after = {0, 0, DmaState::SUSPENDED, 0, 1};
	left.dma_s2mm3.descriptors.push_back(desc_s2mm3_wait_before);
	left.dma_s2mm3.descriptors.push_back(desc_s2mm3_transfer);
	left.dma_s2mm3.descriptors.push_back(desc_s2mm3_sus_after);

	std::cout << "\nsource ram: " << std::endl;
	print_ram(ram, BIG_RAM_SIZE);
	std::cout << "destination ram 1: " << std::endl;
	print_ram(ram1, SMALL_RAM_SIZE);
	std::cout << "destination ram 2: " << std::endl;
	print_ram(ram2, SMALL_RAM_SIZE);
	std::cout << "destination ram 3: " << std::endl;
	print_ram(ram3, SMALL_RAM_SIZE);

	clk.write(0);
	reset.write(0);
	enable.write(1);

	// reset to initialize
	reset.write(1);
	sc_start(1, SC_NS);
	reset.write(0);

	// reassign all initial descriptors, must be done after a reset
	left.dma_mm2s.descriptors[0].state = DmaState::TRANSFER;
	left.dma_s2mm1.descriptors[0].state = DmaState::WAIT;
	left.dma_s2mm2.descriptors[0].state = DmaState::WAIT;
	left.dma_s2mm3.descriptors[0].state = DmaState::WAIT;

	// start transfer of data
	for (i = 0; i < BIG_RAM_SIZE + 1; i++)
	{
		clk.write(1);
		sc_start(1, SC_NS);
		clk.write(0);
		sc_start(1, SC_NS);
	}

	std::cout << "\nsource ram: " << std::endl;
	print_ram(ram, BIG_RAM_SIZE);
	std::cout << "destination ram 1: " << std::endl;
	print_ram(ram1, SMALL_RAM_SIZE);
	std::cout << "destination ram 2: " << std::endl;
	print_ram(ram2, SMALL_RAM_SIZE);
	std::cout << "destination ram 3: " << std::endl;
	print_ram(ram3, SMALL_RAM_SIZE);

	return 0;

}
