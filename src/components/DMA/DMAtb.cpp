#include "DMA.cpp"
#include <systemc.h>
#include <cstdlib>
#include "DMAtb.h"

#define SIM_CYCLES 100

void dma_test::stimulus()
{

	// clock counter, number of Descriptors
	int i,numD;
	
	// Test DMA as a MM2S and S2MM
	int dir;
	for(dir = (int)DmaDirection::MM2S; dir <= (int)DmaDirection::S2MM; dir++)
	{
		// Switch DMA to S2MM on second go around
		if(dir == (int)DmaDirection::MM2S)
		{
			cout << "DMA is set as MM2S" << endl;
		}		
		else
		{
			(*dma).direction = DmaDirection::S2MM;
			cout << "DMA is set as S2MM" << endl;
		}
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
				if(dir == (int)DmaDirection::S2MM)
					stream = load_stream[i];
				clk.write(1);
				sc_start(1, SC_NS);
				clk.write(0);
				sc_start(1, SC_NS);
				
				// If descriptor state is:
				// Waiting: Stream should be same as last time
				// Tranmitting: Stream should be RAM[xcount,xcount + ]
				// Suspended: Stream should remain same
	
				test_dma((*dma).direction, numD, i);
				//cout << endl;
			}
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

void dma_test::test_dma(DmaDirection dir, int numD, int cycle)
{
	// Size of RAM
	int size = (sizeof(*ram)/sizeof(float));
	// If in TRANSFER STATE
	if((*dma).descriptors[numD].state == DmaState::TRANSFER)
	{
		// Check if stream value is correctly accessed
		ram_access = ram[(*dma).descriptors[numD].start + ((*dma).descriptors[numD].x_modify * cycle)];
		
		if (dir == DmaDirection::MM2S)
		{
			// Check stream against actual
			if(stream.read() == ram_access)
					cout << "Test Passed: Stream successfully accessed as " << ram_access << endl;
			else // Stream was not written with correct value
			{
				cout << "Test Failed: DMA accessed incorrect data: " << ram_access << endl;	
				fails ++;
			}
			
			// Check rest of RAM
			if (ram_change(size))
			{
				cout << "Test Failed: Memory Corruption     <<=======================================" << endl;
				fails++;
				print_ram();
			}
				
		}

		else //S2MM
		{
			// First check the intended Memory location
			if (stream.read() == ram_access) // Check that Stream was written to RAM
				cout << "Test Passed: RAM successfully written from Stream" << endl;
			else // RAM differs from Stream
			{
				cout << "Test Failed: RAM written incorrectly  <<========================================" << endl;
				fails++;
			}
			// Check rest of ram
			if (ram_change(size,(*dma).descriptors[numD].start + cycle * (*dma).descriptors[numD].x_modify))
			{
				cout << "Test Failed: Memory Corruption     <<=======================================" << endl;
				fails++;
				print_ram();
			}
		}
	}
			
	// If in SUSPENDED or WAIT state
	else//((*dma).descriptors[numD].state == DmaState::WAIT || (*dma).descriptors[numD].state == DmaState::SUSPENDED)
	{
		if(dir == DmaDirection::MM2S)
		{
			if(last_stream == stream.read())
				cout << "Test passed: Stream unchanged from last access" << endl;
			else
			{
				fails ++;
				cout << "Test Failed: Stream changed since last access  <<=========================================" << endl;
			}
		}
		else // S2MM
			if(ram_change(size))
			{
				cout << "Test Failed: Memory Changed since last access <<==========================================" << endl;
				fails ++;
			}
			else // RAM stayted consistent
				cout << "Test Passed: Memory Unchanaged from last access" << endl;
	}
	update_old_ram();
	last_stream = stream.read();
}

// Check for a change in ram from last cycle ignoring an index if necessary
bool dma_test::ram_change(int size, int index)
{
	int i;
	for(i = 0; i < 100; i++)
	{
		// print RAM for debugging
		//print_ram();
		
		// Ignore address that is inteded to change
		if(i == index)
			continue;
		if(old_ram[i] != ram[i])
		{
			cout << "@" << i << ": " << ram[i] << " instead of old: " << old_ram[i] << endl;
			return 1;
		}
	}
	return 0;
}

// Check for a change in ram from last cycle
bool dma_test::ram_change(int size)
{
	int i;
	for(i = 0; i < 100; i++)
	{
		if(old_ram[i] != ram[i])
		{
			cout << "@" << i << ": " << ram[i] << " instead of old: " << old_ram[i] << endl;
			return 1;
		}
	}
	return 0;
}

void dma_test::update_old_ram(void)
{
	int i;
	for(i = 0; i < 100; i++)
		old_ram[i] = ram[i];
}

// Help Debugging by printing RAM
void dma_test::print_ram(void)
{
	cout << " CURRENT RAM " << endl;
	cout << "= = = = = = = = = =" << endl;
	int i, size = sizeof(*ram)/sizeof(float);
	for(i = 0; i < 100; i++)
	{
		if(!(i % 10))
			cout << endl;
		cout << ram[i] << " ";
	}
	cout << endl << "= = = = = = = = = =" << endl << endl;

	cout << " OLD RAM " << endl;
	cout << "= = = = = = = = = =" << endl;
	for(i = 0; i < 100; i++)
	{
		if(!(i % 10))
			cout << endl;
		cout << old_ram[i] << " ";
	}
	cout << endl << "= = = = = = = = = =" << endl << endl;
}

// This will just print information about the tests that will run on the DMA
void dma_test::print_test_info()
{
	cout << "Testbench includes following tests:" << endl;


}
