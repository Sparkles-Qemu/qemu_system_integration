#include "DMA.cpp"
#include "systemc.h"
#include <cstdlib>
#include "DMAtb.h"

#define SIM_CYCLES 100

void dma_test::stimulus()
{
	// clock counter, number of Descriptors
	int i,numD;

	// Track last stream and actual ram data
	float last_stream;
	float ram_access;

	// Track fails
	int fails = 0;

	// Enable DMA
	reset.write(0);
	enable.write(0);
	clk.write(0);

	cout << "===== Test Bench =====" << endl;
	cout << endl << "Test 1: Reset DMA" << endl;

	sc_start(1, SC_NS);

	// Reset DMA
	enable.write(1);
	reset.write(1);
	
	// Run 5 cycles then set reset low again
	for (i = 0; i <= 5; i++)
	{
		clk.write(1);
		sc_start(1, SC_NS);
		clk.write(0);
		sc_start(1, SC_NS);
	};
	// Deassert reset to begin initial transfer
	reset.write(0);
	
	// Run Descriptors
	cout << endl << "Test 2: Run Descriptors" << endl;

	// "enable" first descriptor
	(*dma).descriptors[0].state = DmaState::TRANSFER;
	
	// Each Descriptor is run and tested
	for(numD = 0; numD < (*dma).descriptors.size(); numD++)
	{
		int cycles = (*dma).descriptors[numD].x_count;
		cout << "Computing Descriptor " << numD << " for " << cycles << " cycles" << endl;
		for(i = 0; i < cycles; i++)
		{
			clk.write(1);
			sc_start(1, SC_NS);
			clk.write(0);
			sc_start(1, SC_NS);
			
			// If descriptor state is:
			// Waiting: Stream should be same as last time
			// Tranmitting: Stream should be RAM[xcount,xcount + ]
			// Suspended: Stream should remain same
			
			// If in TRANSFER STATE
			if((*dma).descriptors[numD].state == DmaState::TRANSFER)
			{
				// Check if stream value is correctly accessed
				ram_access = ram[(*dma).descriptors[numD].start + ((*dma).descriptors[numD].x_modify * i)];
				
				// Check stream against actual
				if(stream.read() == ram_access)
					cout << "Test Passed: Stream successfully accessed as " << ram_access << endl;
				else
				{
					cout << "Test Failed: DMA accessed incorrect data: " << ram_access << endl;	
					fails ++;
				}
			}
			
			// If in SUSPENDED or WAIT state
			else//((*dma).descriptors[numD].state == DmaState::WAIT || (*dma).descriptors[numD].state == DmaState::SUSPENDED)
			{
				if(last_stream == stream.read())
					cout << "Test passed: Stream unchanged from last access" << endl;
				else
				{
					fails ++;
					cout << "Test Failed: Stream changed since last access" << endl;
				}
			}
			last_stream = stream.read();	
			cout << endl;
		}

	}


	// Test Async Reset
	cout << endl<< "Test 3: Test Async Reset" << endl;
	//cout << "@ " << sc_time_stamp() << " Done with compute, testing async reset" << endl;

	sc_start(1.5, SC_NS);
	reset.write(1);
	sc_start(1.5, SC_NS);

	cout << "===== Test Bench Complete =====" << endl << fails << " Failed Access Attempts" << endl << "Terminating Test Module" << endl;
}

// This will just print information about the tests that will run on the DMA
void dma_test::print_test_info()
{
	cout << "Testbench includes following tests:" << endl;


}
