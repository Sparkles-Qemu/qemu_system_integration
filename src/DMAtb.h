// DMAtb.h
//

#include "systemc.h"
//#include "DMA.cpp"

SC_MODULE(dma_test)
{
	sc_out<bool> clk;
	sc_out<bool> enable;
	sc_out<bool> reset;
	sc_inout<float> stream;	
	// Initialize and Connect DMA Device
	//DMA dma("dma_mm2s",DmaDirection::MM2S,clk,reset,enable,ram,stream);

	// Point to tracefile in constructor
	sc_trace_file *wf;

	// Call stimulus
	//void stimulus();
	
	SC_CTOR(dma_test):
		clk("clk"),
		enable("enable"),
		reset("reset"),
		stream("stream")
		//ram(ram)
		//dma("dma")
	{
		//SC_THREAD(dma_test);
		//sensitive << clk.pos();
		//sensitive << reset.pos();
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
