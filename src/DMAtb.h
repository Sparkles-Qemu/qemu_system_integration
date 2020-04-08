// DMAtb.h
//

#include "systemc.h"
#include "DMA.cpp"

SC_MODULE(dma_test)
{
	sc_signal<bool> clk;
	sc_signal<bool> enable;
	sc_signal<bool> reset;
	sc_signal<float, SC_MANY_WRITERS> stream;	
	float ram[100];

	// Initialize and Connect DMA Device
	//DMA dma("dma_mm2s",DmaDirection::MM2S,clk,reset,enable,ram,stream);
	struct DMA *dma;

	// Point to tracefile in constructor
	sc_trace_file *wf;

	// Initialize stimulus function
	void stimulus();
	void print_test_info();

	SC_CTOR(dma_test)//:
		//clk("clk"),
		//enable("enable"),
		//reset("reset"),
		//stream("stream")
		//ram(ram)
		//dma("dma")
	{
		SC_METHOD(stimulus);
		//sensitive << clk.pos();
		//sensitive << reset.pos();
		
		//*dma = DMA ("dma_mm2s",DmaDirection::MM2S,clk,reset,enable,ram,stream);
		//*dma = dma1;
		
		// DMA instantiation
		DMA dma_temp("dma_mm2s", DmaDirection::MM2S, clk, reset, enable, ram, stream);
		Descriptor d1 = {1, 0, DmaState::SUSPENDED, 3, 1}; // Test Suspended State
		Descriptor d2 = {2, 50, DmaState::WAIT, 5, 2};     // Test Waiting State
		Descriptor d3 = {3, 10, DmaState::TRANSFER, 2, 10};// Test Standard access
		Descriptor d4 = {0, 90, DmaState::TRANSFER, 11, 1};// This should overflow ram
		dma_temp.descriptors.push_back(d1);
		dma_temp.descriptors.push_back(d2);
		dma_temp.descriptors.push_back(d3);
		dma_temp.descriptors.push_back(d4);
		dma_temp.print_descriptors();
		dma = &dma_temp;
		
		// Define and identify signals, this is done in the dma constructor?
		//dma.clk(clk);
		//dma.enable(enable);
		//dma.reset(reset);

		wf = sc_create_vcd_trace_file("./traces/sim_signals.trace");
	
		// Dump signals
		sc_trace(wf, clock, "clock");
		sc_trace(wf, reset, "reset");
		sc_trace(wf, enable, "enable");
		sc_trace(wf, stream, "stream");
		//sc_trace(wf, *ram, "RAM");

	}
	// Close trace file on simulation end
	~dma_test()
	{
		sc_close_vcd_trace_file(wf);
	}
};

