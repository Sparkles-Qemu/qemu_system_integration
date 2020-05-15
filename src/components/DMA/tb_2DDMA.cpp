#include <systemc.h>
#include "DMA.cpp"

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
	sc_signal<float, SC_MANY_WRITERS> stream("stream"); // AC_MANY_WRITERS allows stream to have numerous drivers, (dma and dma_test)

	// RAM representing external Memory
	float ram[1000] = {0};

	// Fill ram with 1-100
	for (int i = 0; i < 100; i++)
	{
		ram[i] = i;
	}

	// DMA for MM2S S2MM instantiation
	DMA_2D dma_MM2S("DMA_2D_MM2S", DmaDirection::MM2S, clk, reset, enable, ram, stream);
	DMA_2D dma_S2MM("DMA_2D_S2MM", DmaDirection::S2MM, clk, reset, enable, ram, stream);

	// 1D transfer test
	unsigned int transfer_1D_blob_size = 10;
	Descriptor_2D desc_s2mm_Transfer_1D_transfer = {2, 0, DmaState::TRANSFER, transfer_1D_blob_size, 1, 0, 0};
	Descriptor_2D desc_s2mm_Transfer_1D_suspend_after = {0, 0, DmaState::SUSPENDED, 0, 1};

	
	Descriptor_2D desc_mm2s_Transfer_1D_wait_before = {1, 0, DmaState::WAIT, 1, 1, 0, 0};
	Descriptor_2D desc_mm2s_Transfer_1D_transfer = {2, 0, DmaState::TRANSFER, transfer_1D_blob_size, 1, 0, 0};
	Descriptor_2D desc_mm2s_Transfer_1D_suspend_after = {0, 0, DmaState::SUSPENDED, 0, 1};

	// Start Programming Sequence
	dma_MM2S.loadProgram({desc_s2mm_Transfer_1D_transfer,
						  desc_s2mm_Transfer_1D_suspend_after});

	dma_S2MM.loadProgram({desc_mm2s_Transfer_1D_wait_before,
						  desc_mm2s_Transfer_1D_transfer,
						  desc_mm2s_Transfer_1D_suspend_after});




	//2D transfer test

	//1D Strided Transfer Test

	//2D Strided Transfer Test

	return 0;
}
