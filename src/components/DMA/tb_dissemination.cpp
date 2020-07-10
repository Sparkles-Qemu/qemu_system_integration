#include <systemc.h>
#include "DMA_2D.cpp"

using std::cout;
using std::endl;

#define MAX_RESET_CYCLES 10
#define MAX_SIM_CYCLES 20

float ram[1000] = {0};

sc_signal<bool> clk("clk");
sc_signal<bool> reset("reset");
sc_signal<bool> enable("enable");
sc_signal<bool> program_mode("program_mode");
sc_signal<float, SC_MANY_WRITERS> stream("stream"); // AC_MANY_WRITERS allows stream to have numerous drivers, (dma and dma_test)

sc_trace_file *tf = sc_create_vcd_trace_file("Prog_trace");

// DMA for MM2S S2MM instantiation
DMA_2D dma_2d_mm2s("DMA_2D_MM2S", DmaDirection::MM2S, clk, reset, enable, program_mode, ram, stream, 0, 6, 5, 0, 0, 1, true, tf);
DMA_2D dma_2d_s2mm("DMA_2D_S2MM", DmaDirection::S2MM, clk, reset, enable, program_mode, ram, stream, 1, 0, 5, 1, 500, 1, true, tf);

bool runDisseminationTest()
{
	// RAM representing external Memory
	memset(ram, 0, 1000*sizeof(float));

	cout << "Populating RAM " << endl;

	// Fill ram with 1-100
	for (int i = 0; i < 100; i++)
	{
		ram[i] = i+1;
	}
	
	cout << "Beggining programming sequence " << endl;

	enable = 0;
	reset = 1;
	clk = 0;

	sc_start(1, SC_NS);

	reset = 0;

	sc_start(1, SC_NS);
	
	program_mode = 1;

	for(int i = 0 ; i < 100; i++)
	{
		clk = 1;
		sc_start(0.5, SC_NS);
		clk = 0;
		sc_start(0.5, SC_NS);
	}


	return true;
}

bool run1DTransferTest()
{
	// RAM representing external Memory
	memset(ram, 0, 1000*sizeof(float));

	cout << "Populating RAM " << endl;

	// Fill ram with 1-100
	for (int i = 0; i < 100; i++)
	{
		ram[i] = i;
	}
	
	cout << "Beggining program sequence " << endl;

	enable = 0;
	reset = 1;
	clk = 0;

	sc_start(1, SC_NS);

	reset = 0;

	sc_start(1, SC_NS);

	Descriptor_2D dma_2d_mm2s_transfer = {1, 0, DmaState::TRANSFER, 10, 1, 0, 0}; 	
	Descriptor_2D dma_2d_mm2s_suspend = {1, 0, DmaState::SUSPENDED, 10, 1, 0, 0}; 	

	Descriptor_2D dma_2d_s2mm_wait = {1, 0, DmaState::WAIT, 1, 1, 0, 0};			
	Descriptor_2D dma_2d_s2mm_transfer = {2, 100, DmaState::TRANSFER, 10, 1, 0, 0}; 
	Descriptor_2D dma_2d_s2mm_suspend = {2, 0, DmaState::SUSPENDED, 10, 1, 0, 0};	

	dma_2d_mm2s.loadProgram({dma_2d_mm2s_transfer,
							 dma_2d_mm2s_suspend});

	dma_2d_s2mm.loadProgram({dma_2d_s2mm_wait,
							 dma_2d_s2mm_transfer,
							 dma_2d_s2mm_suspend});
	

	cout << "@" << sc_time_stamp() << " Load Pulse " << endl;

	clk = 1;
	sc_start(0.5, SC_NS);
	clk = 0;
	sc_start(0.5, SC_NS);

	enable = 1;

	cout << "@" << sc_time_stamp() << " Running Transfer " << endl;

	for (unsigned int i = 0; i < 11; i++)
	{
		clk = 1;
		sc_start(0.5, SC_NS);
		clk = 0;
		sc_start(0.5, SC_NS);
	}
	
	cout << "@" << sc_time_stamp() << " Validating Transfer " << endl;

	for (unsigned int i = 0; i < 10; i++)
	{
		if(ram[i] != ram[i+100])
		{
			cout << "@" << sc_time_stamp() << " validation failed :(" << endl;
			
			cout << "@" << sc_time_stamp() << \
			" ram [" << i << "]: " << ram[i] << " != " << "ram[" << i+100 << "]: "  << ram[i+100] << \
			endl;

			return false;
		}
	}
	
	cout << "@" << sc_time_stamp() << " Transfer Complete " << endl;
	cout << "@" << sc_time_stamp() << " 1D Transfer Validation Succuss " << endl;
	
	return true;
}

bool run1DStridedTransferTest()
{
	// RAM representing external Memory
	memset(ram, 0, 1000*sizeof(float));

	cout << "Populating RAM " << endl;

	// Fill ram with 1-100
	for (int i = 0; i < 100; i++)
	{
		ram[i] = i;
	}
	
	cout << "Beggining program sequence " << endl;

	enable = 0;
	reset = 1;
	clk = 0;

	sc_start(1, SC_NS);

	reset = 0;

	sc_start(1, SC_NS);

	Descriptor_2D dma_2d_mm2s_transfer = {1, 0, DmaState::TRANSFER, 10, 2, 0, 0}; 	
	Descriptor_2D dma_2d_mm2s_suspend = {1, 0, DmaState::SUSPENDED, 10, 1, 0, 0}; 	

	Descriptor_2D dma_2d_s2mm_wait = {1, 0, DmaState::WAIT, 1, 1, 0, 0};			
	Descriptor_2D dma_2d_s2mm_transfer = {2, 100, DmaState::TRANSFER, 10, 2, 0, 0}; 
	Descriptor_2D dma_2d_s2mm_suspend = {2, 0, DmaState::SUSPENDED, 10, 1, 0, 0};	

	dma_2d_mm2s.loadProgram({dma_2d_mm2s_transfer,
							 dma_2d_mm2s_suspend});

	dma_2d_s2mm.loadProgram({dma_2d_s2mm_wait,
							 dma_2d_s2mm_transfer,
							 dma_2d_s2mm_suspend});
	

	cout << "@" << sc_time_stamp() << " Load Pulse " << endl;

	clk = 1;
	sc_start(0.5, SC_NS);
	clk = 0;
	sc_start(0.5, SC_NS);

	enable = 1;

	cout << "@" << sc_time_stamp() << " Running Transfer " << endl;

	for (unsigned int i = 0; i < 11; i++)
	{
		clk = 1;
		sc_start(0.5, SC_NS);
		clk = 0;
		sc_start(0.5, SC_NS);
	}
	
	cout << "@" << sc_time_stamp() << " Validating Transfer " << endl;

	for (unsigned int i = 0; i < 10; i+=2)
	{
		if(ram[i] != ram[i+100])
		{
			cout << "@" << sc_time_stamp() << " validation failed :(" << endl;
			
			cout << "@" << sc_time_stamp() << \
			" ram [" << i << "]: " << ram[i] << " != " << "ram[" << i+100 << "]: "  << ram[i+100] << \
			endl;

			return false;
		}
	}
	
	cout << "@" << sc_time_stamp() << " Transfer Complete " << endl;
	cout << "@" << sc_time_stamp() << " 1D Strided tTransfer Validation Succuss " << endl;
	
	return true;
}


bool run2DTransferTest()
{
	// RAM representing external Memory
	memset(ram, 0, 1000*sizeof(float));

	cout << "Populating RAM " << endl;

	// Fill ram with 1-100
	for (int i = 0; i < 100; i++)
	{
		ram[i] = i;
	}
	
	cout << "Beggining program sequence " << endl;

	enable = 0;
	reset = 1;
	clk = 0;

	sc_start(1, SC_NS);

	reset = 0;

	sc_start(1, SC_NS);

	Descriptor_2D dma_2d_mm2s_transfer = {1, 0, DmaState::TRANSFER, 10, 1, 10, 1}; 	
	Descriptor_2D dma_2d_mm2s_suspend = {1, 0, DmaState::SUSPENDED, 10, 1, 0, 0}; 	

	Descriptor_2D dma_2d_s2mm_wait = {1, 0, DmaState::WAIT, 1, 1, 0, 0};			
	Descriptor_2D dma_2d_s2mm_transfer = {2, 100, DmaState::TRANSFER, 10, 1, 10, 1}; 
	Descriptor_2D dma_2d_s2mm_suspend = {2, 0, DmaState::SUSPENDED, 10, 1, 0, 0};	

	dma_2d_mm2s.loadProgram({dma_2d_mm2s_transfer,
							 dma_2d_mm2s_suspend});

	dma_2d_s2mm.loadProgram({dma_2d_s2mm_wait,
							 dma_2d_s2mm_transfer,
							 dma_2d_s2mm_suspend});
	

	cout << "@" << sc_time_stamp() << " Load Pulse " << endl;

	clk = 1;
	sc_start(0.5, SC_NS);
	clk = 0;
	sc_start(0.5, SC_NS);

	enable = 1;

	cout << "@" << sc_time_stamp() << " Running Transfer " << endl;

	for (unsigned int i = 0; i < 101; i++)
	{
		clk = 1;
		sc_start(0.5, SC_NS);
		clk = 0;
		sc_start(0.5, SC_NS);
		// cout << "@" << sc_time_stamp() << " Streamout " << stream << endl;

	}
	
	cout << "@" << sc_time_stamp() << " Validating Transfer " << endl;

	for (unsigned int i = 0; i < 100; i++)
	{
		if(ram[i] != ram[i+100])
		{
			cout << "@" << sc_time_stamp() << " validation failed :(" << endl;
			
			cout << "@" << sc_time_stamp() << \
			" ram [" << i << "]: " << ram[i] << " != " << "ram[" << i+100 << "]: "  << ram[i+100] << \
			endl;

			return false;
		}
	}
	
	cout << "@" << sc_time_stamp() << " Transfer Complete " << endl;
	cout << "@" << sc_time_stamp() << " 2D Transfer Validation Succuss " << endl;
	
	return true;
}

bool run2DStridedTransferTest()
{
	// RAM representing external Memory
	memset(ram, 0, 1000*sizeof(float));

	cout << "Populating RAM " << endl;

	// Fill ram with 1-100
	for (int i = 0; i < 100; i++)
	{
		ram[i] = i+1;
	}
	
	cout << " Beginning program sequence " << endl;

	enable = 0;
	reset = 1;
	clk = 0;

	sc_start(1, SC_NS);

	reset = 0;

	sc_start(1, SC_NS);

	Descriptor_2D dma_2d_mm2s_transfer = {1, 0, DmaState::TRANSFER, 5, 2, 4, 12}; 	
	Descriptor_2D dma_2d_mm2s_suspend = {1, 0, DmaState::SUSPENDED, 10, 1, 0, 0}; 	

	Descriptor_2D dma_2d_s2mm_wait = {1, 0, DmaState::WAIT, 1, 1, 0, 0};			
	Descriptor_2D dma_2d_s2mm_transfer = {2, 100, DmaState::TRANSFER, 5, 2, 4, 12}; 
	Descriptor_2D dma_2d_s2mm_suspend = {2, 0, DmaState::SUSPENDED, 10, 1, 0, 0};	

	dma_2d_mm2s.loadProgram({dma_2d_mm2s_transfer,
							 dma_2d_mm2s_suspend});

	dma_2d_s2mm.loadProgram({dma_2d_s2mm_wait,
							 dma_2d_s2mm_transfer,
							 dma_2d_s2mm_suspend});
	

	cout << "@" << sc_time_stamp() << " Load Pulse " << endl;

	clk = 1;
	sc_start(0.5, SC_NS);
	clk = 0;
	sc_start(0.5, SC_NS);

	enable = 1;

	cout << "@" << sc_time_stamp() << " Running Transfer " << endl;

	for (unsigned int i = 0; i < 51; i++)
	{
		// cout << "@" << sc_time_stamp() << " current_index " << dma_2d_mm2s.current_ram_index << endl;
		clk = 1;
		sc_start(0.5, SC_NS);
		clk = 0;
		sc_start(0.5, SC_NS);
		// cout << "@" << sc_time_stamp() << " Streamout " << stream << endl;
	}
	
	cout << "@" << sc_time_stamp() << " Validating Transfer " << endl;

	for (unsigned int j = 0; j < 100; j+=10)
	{
		for (unsigned int i = j; i < j+10; i+=2)
		{

			if(ram[i] != ram[i+100])
			{
				cout << "@" << sc_time_stamp() << " validation failed :(" << endl;
				cout << "@" << sc_time_stamp() << \
				" ram [" << i << "]: " << ram[i] << " != " << "ram[" << i+100 << "]: "  << ram[i+100] << \
				endl;

				return false;
			}
		}
		j=j+10;
	}
	
	cout << "@" << sc_time_stamp() << " Transfer Complete " << endl;
	cout << "@" << sc_time_stamp() << " 2D Strided Transfer Validation Succuss " << endl;
	
	return true;
}

void printBreak()
{
	for (unsigned int i = 0; i < 20; i++)
	{
		cout << " - ";
	}
	cout << endl;
	
}


int sc_main(int argc, char *argv[])
{	
	 tf->set_time_unit(1, SC_PS);

	// printBreak();
	// cout << "Starting 1D Test " << endl;
	// if(!run1DTransferTest())
	// {
	// 	cout << " 1D Transfer Validation FAIL :( " << endl;
	// 	return -1;
	// }

	// printBreak();
	// cout << "Starting 2D Test " << endl;
	// if(!run2DTransferTest())
	// {
	// 	cout << " 2D Transfer Validation FAIL :( " << endl;
	// 	return -1;
	// }

	// printBreak();
	// cout << "Starting 1D Strided Test " << endl;
	// if(!run1DStridedTransferTest())
	// {
	// 	cout << " 1D Strided Transfer Validation FAIL :( " << endl;
	// 	return -1;
	// }

	// printBreak();
	// cout << "Starting 2D Strided Test " << endl;printBreak();
	// cout << "Starting 1D Test " << endl;
	// if(!run1DTransferTest())
	// {
	// 	cout << " 1D Transfer Validation FAIL :( " << endl;
	// 	return -1;
	// }

	// printBreak();
	// cout << "Starting 2D Test " << endl;
	// if(!run2DTransferTest())
	// {
	// 	cout << " 2D Transfer Validation FAIL :( " << endl;
	// 	return -1;
	// }

	// printBreak();
	// cout << "Starting 1D Strided Test " << endl;
	// if(!run1DStridedTransferTest())
	// {
	// 	cout << " 1D Strided Transfer Validation FAIL :( " << endl;
	// if(!run2DStridedTransferTest())
	// {
	// 	cout << " 2D Strided Transfer Validation FAIL :( " << endl;
	// 	return -1;
	// }

	printBreak();
	runDisseminationTest();


	cout << "TEST BENCH SUCCESS " << endl << endl;


    cout << "       aOOOOOOOOOOa" << endl;
    cout << "     aOOOOOOOOOOOOOOa" << endl;
    cout << "   aOO    OOOOOO    OOa" << endl;
    cout << "  aOOOOOOOOOOOOOOOOOOOa" << endl;
    cout << " aOOOOO   OOOOOO   OOOOOa" << endl;
    cout << "aOOOOO     OOOO     OOOOOa" << endl;
    cout << "aOOOOOOOOOOOOOOOOOOOOOOOOa" << endl;
    cout << "aOOOOOOOOOOOOOOOOOOOOOOOOa" << endl;
    cout << "aOOOOO   OOOOOOOO   OOOOOa" << endl;
    cout << " aOOOOO    OOOO    OOOOOa" << endl;
    cout << "  aOOOOO          OOOOOa" << endl;
    cout << "   aOOOOOOOOOOOOOOOOOOa" << endl;
    cout << "     aOOOOOOOOOOOOOOa" << endl;
    cout << "       aOOOOOOOOOOa" << endl;

	
	return 0;
}
