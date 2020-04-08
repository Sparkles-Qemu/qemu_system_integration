#include "DMA.cpp"
#include "systemc.h"
#include <cstdlib>
#include "DMAtb.h"

#define SIM_CYCLES 100

void dma_test::stimulus()
{
	// clock counter
	int i;

	// Enable DMA
	reset.write(0);
	enable.write(0);
	clk.write(0);

	cout << "===== Test Bench =====" << endl;
	cout << "Test 1: Reset DMA" << endl;

	cout << "sc_start" << endl;	
	sc_start(1, SC_NS);

	// Reset DMA
	reset.write(1);
	cout << "write reset 1" << endl;
	for (i = 0; i <= 0; i++)
	{
		//cout << clk.read() << endl;
		clk.write(1);
		sc_start(1, SC_NS);
		clk.write(0);
		sc_start(1, SC_NS);
		cout << "endloop" << endl;
	};
	// Deassert reset to begin initial transfer
	reset.write(0);
	
	// Run Descriptors
	cout << "Test 2: Run Descriptors" << endl;

	cout << "@ " << sc_time_stamp() << " Start Compute" << endl;
	for (i = 0; i <= SIM_CYCLES; i++)
        {
                clk.write(1);
                sc_start(1, SC_NS);
                clk.write(0);
                sc_start(1, SC_NS);
        };

	// Test Async Reset
	cout << "@ " << sc_time_stamp() << " Done with compute, testing async reset" << endl;

	sc_start(1.5, SC_NS);
	reset.write(1);
	sc_start(1.5, SC_NS);

	cout << "@ " << sc_time_stamp() << " Test Bench complete, Terminating test module..." << endl;
}

// This will just print information about the tests that will run on the DMA
void dma_test::print_test_info()
{
	cout << "Testbench includes following tests:" << endl;


}
SC_HAS_PROCESS(dma_test);

//SC_MODULE_EXPORT(dma_test)	
/*
int sc_main(int argc, char* argv[]){
	sc_signal<bool> clk("clk");
	sc_signal<bool> reset("reset");
	sc_signal<bool> enable("enable");
	sc_signal<float> stream("stream");
	
	int i,j,k;

	// Fill RAM with 1-100
	int size = 100;
	float ram[size];
	for (i = 0; i < size; i++)
		ram[i] = i;

	// Variables used to fill Descriptors
	int numStates = 3,numDescriptors = 10,xCount,xModify,ram_address;
	DmaState state;

	// Initialize and Connect DMA Device
	DMA dma("MM2S",DmaDirection::MM2S,clk,reset,enable,ram,stream);
	
	// Build Descriptors
	Descriptor d;
	int next = 1;
	for(i = 0; i < numDescriptors; i++){
		DmaState state = DmaState((i % 2) + 1);
		xCount =  (rand() % 10) + 1;
		xModify =  (rand() % 3) + 1;
		ram_address =  (rand() % 100) + 1;
		
		d = {next,ram_address,state,xCount,xModify};
		dma.descriptors.push_back(d);
		next++;
	}
	dma.print_descriptors();
<F7>
	// Do we need to test multiple devices at the same time?
	//s2mm DMA("S2MM");
	
	// Open VCD file
	sc_trace_file *wf = sc_create_vcd_trace_file("./traces/sim_signals.trace");

	// Dump signals
	sc_trace(wf, clock, "clock");
	sc_trace(wf, reset, "reset");
	sc_trace(wf, enable, "enable");
	sc_trace(wf, stream, "stream");
	sc_trace(wf, *ram, "RAM");

	// Start Scheduler
	sc_start(1, SC_NS);

	// Initialize variables
	reset = 0;
	enable = 0;
	
	// Run clock, w/ enable = 0, stream = 0
	for(i = 0; i < 10; i++){
		clk = 0;
		sc_start(1, SC_NS);
		clk = 1;
		sc_start(1, SC_NS);
	}

	// Enable DMA
	enable = 1;
	cout << "@ " << sc_time_stamp() << " Asserting Enable" << endl;
	
	for(i = 0; i < SIM_CYCLES; i++){
		clk = 1;
		sc_start(1, SC_NS);
		clk = 0;
		sc_start(1, SC_NS);
	}

	//cout << "@ " << sc_time_stamp() << " Done with compute, testing async reset" << endl;

	 
	cout << "@ " << sc_time_stamp() << " Sim complete, Simulation terminating .... " << endl;	

	return 0;

}

*/


	
