#include "systemc.h"
#include "DMA.cpp"
#include "DMAtb.h"

using std::cout;
using std::endl;

#define MAX_RESET_CYCLES 10
#define MAX_SIM_CYCLES 20

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
	sc_signal<float, SC_MANY_WRITERS> stream("stream");	// AC_MANY_WRITERS allows stream to have numerous drivers, (dma and dma_test)
	
	// RAM representing external Memory
	float ram[100];

	// Fill ram with 1-100
	int i;
	for (i = 0; i < 100; i++)
		ram[i] = i;

	// DMA for MM2S S2MM instantiation
	DMA dma_temp("dma_temp", DmaDirection::MM2S, clk, reset, enable, ram, stream);
	Descriptor d1 = {1, 0, DmaState::SUSPENDED, 3, 1}; // Test Suspended State
	Descriptor d2 = {2, 50, DmaState::WAIT, 5, 2};     // Test Waiting State
	Descriptor d3 = {3, 10, DmaState::TRANSFER, 2, 10};// Test Standard access
	Descriptor d4 = {0, 89, DmaState::TRANSFER, 11, 1};// This should overflow ram
	dma_temp.descriptors.push_back(d1);
	dma_temp.descriptors.push_back(d2);
	dma_temp.descriptors.push_back(d3);
	dma_temp.descriptors.push_back(d4);
	dma_temp.print_descriptors();
	
	// DMA MM2S and S2MM Testbench
	dma_test test("dma_test", dma_temp, ram);
	test.clk(clk);
	test.enable(enable);
	test.reset(reset);
	test.stream(stream);

	// DMA_MM2MM instantiation
	DMA dma_s2mm("dma_s2mm", DmaDirection::S2MM, clk, reset, enable, ram, stream);
	DMA dma_mm2s("dma_mm2s", DmaDirection::MM2S, clk, reset, enable, ram, stream);
	dma_s2mm.descriptors.push_back(d1);
        dma_s2mm.descriptors.push_back(d2);
        dma_s2mm.descriptors.push_back(d3);
        dma_s2mm.descriptors.push_back(d4);
	dma_mm2s.descriptors.push_back(d1);
        dma_mm2s.descriptors.push_back(d2);
        dma_mm2s.descriptors.push_back(d3);
        dma_mm2s.descriptors.push_back(d4);

	DMA_MM2MM dma_mm2mm("dma_mm2mm", dma_s2mm, dma_mm2s, clk, reset, enable);
	
	dma_mm2mm.mm2s->print_descriptors();
	dma_mm2mm.s2mm->print_descriptors();

	sc_start();//1, SC_NS); // Run Test

	return 0;
}


