// DMAtb.h
//

#include "systemc.h"
#include "DMA.cpp"

SC_MODULE(dma_test)
{
	sc_out<bool> clk;
	sc_out<bool> enable;
	sc_out<bool> reset;
	sc_inout<float> stream;	
	
	float *ram;
	
	// Initialize and Connect DMA Device
	DMA *dma;

	// Point to tracefile in constructor
	sc_trace_file *wf;

	// Initialize stimulus function
	void stimulus();
	void print_test_info();

	// Called because we are not using default constructor
	SC_HAS_PROCESS(dma_test);

	// dma_test Constructor with dma input
	dma_test(sc_module_name name, DMA &dma_in, float *external_ram)
	{
		dma = &dma_in;
		SC_METHOD(stimulus);
		
		// Set ram pointer
		ram = external_ram;

		//cout << descriptors[1] << endl;
		//cout << "Descriptors: " << dma_in.descriptors[1].x_count << endl;

		// Point to trace file
		wf = sc_create_vcd_trace_file("./traces/sim_signals.trace");
	
		// Dump signals
		sc_trace(wf, clock, "clock");
		sc_trace(wf, reset, "reset");
		sc_trace(wf, enable, "enable");
		sc_trace(wf, stream, "stream");
	}
	// Close trace file on simulation end
	~dma_test()
	{
		sc_close_vcd_trace_file(wf);
	}
};

