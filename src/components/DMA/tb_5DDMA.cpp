#include <systemc.h>
#include "DMA_5D.cpp"

using std::cout;
using std::endl;

#define MAX_RESET_CYCLES 10
#define MAX_SIM_CYCLES 20

float ram[1000] = {0};

sc_signal<bool> clk("clk");
sc_signal<bool> reset("reset");
sc_signal<bool> enable("enable");
sc_signal<float, SC_MANY_WRITERS> stream("stream"); // AC_MANY_WRITERS allows stream to have numerous drivers, (dma and dma_test)

// DMA for MM2S S2MM instantiation
DMA_5D dma_5d_mm2s("DMA_5D_MM2S", DmaDirection::MM2S, clk, reset, enable, ram, stream);
DMA_5D dma_5d_s2mm("DMA_5D_S2MM", DmaDirection::S2MM, clk, reset, enable, ram, stream);

bool runIm2ColTest()
{
	// RAM representing external Memory
	memset(ram, 0, 1000 * sizeof(float));

	std::cout << "Populating RAM " << std::endl;

	// Fill ram with 1-100
	for (int i = 0; i < 10 * 10 * 3; i++)
	{
		ram[i] = i + 1;
	}

	std::cout << " Beginning program sequence " << std::endl;

	enable = 0;
	reset = 1;
	clk = 0;

	sc_start(1, SC_NS);

	reset = 0;

	sc_start(1, SC_NS);

	Descriptor_5D dma_5d_mm2s_transfer = {1, 0, DmaState::TRANSFER, 3, 1, 2, 8, 2, 78, 7, -221, 7, -19};
	Descriptor_5D dma_5d_mm2s_suspend = {1, 0, DmaState::SUSPENDED, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	Descriptor_5D dma_5d_s2mm_wait = {1, 0, DmaState::WAIT, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
	Descriptor_5D dma_5d_s2mm_transfer = {2, 100, DmaState::TRANSFER, 5, 2, 4, 12, 0, 0, 0, 0, 0, 0};
	Descriptor_5D dma_5d_s2mm_suspend = {2, 0, DmaState::SUSPENDED, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	dma_5d_mm2s.loadProgram({dma_5d_mm2s_transfer,
							 dma_5d_mm2s_suspend});

	// dma_5d_s2mm.loadProgram({dma_5d_s2mm_wait,
	// 						 dma_5d_s2mm_transfer,
	// 						 dma_5d_s2mm_suspend});

	std::cout << "@" << sc_time_stamp() << " Load Pulse " << std::endl;

	clk = 1;
	sc_start(0.5, SC_NS);
	clk = 0;
	sc_start(0.5, SC_NS);

	enable = 1;

	std::cout << "@" << sc_time_stamp() << " Running Transfer " << std::endl;

	for (unsigned int i = 0; i < 51; i++)
	{
		clk = 1;
		sc_start(0.5, SC_NS);
		clk = 0;
		sc_start(0.5, SC_NS);
		std::cout << "@" << sc_time_stamp() << " Stream Out " << stream << std::endl;
	}

	// std::cout << "@" << sc_time_stamp() << " Validating Transfer " << std::endl;

	// for (unsigned int j = 0; j < 100; j+=10)
	// {
	// 	for (unsigned int i = j; i < j+10; i+=2)
	// 	{

	// 		if(ram[i] != ram[i+100])
	// 		{
	// 			std::cout << "@" << sc_time_stamp() << " validation failed :(" << std::endl;
	// 			std::cout << "@" << sc_time_stamp() << \
	// 			" ram [" << i << "]: " << ram[i] << " != " << "ram[" << i+100 << "]: "  << ram[i+100] << \
	// 			std::endl;

	// 			return false;
	// 		}
	// 	}
	// 	j=j+10;
	// }

	// std::cout << "@" << sc_time_stamp() << " Transfer Complete " << std::endl;
	// std::cout << "@" << sc_time_stamp() << " 2D Strided Transfer Validation Succuss " << std::endl;

	return true;
}

bool run2DStridedTransferTest()
{
	// RAM representing external Memory
	memset(ram, 0, 1000 * sizeof(float));

	std::cout << "Populating RAM " << std::endl;

	// Fill ram with 1-100
	for (int i = 0; i < 100; i++)
	{
		ram[i] = i + 1;
	}

	std::cout << " Beginning program sequence " << std::endl;

	enable = 0;
	reset = 1;
	clk = 0;

	sc_start(1, SC_NS);

	reset = 0;

	sc_start(1, SC_NS);

	Descriptor_5D dma_5d_mm2s_transfer = {1, 0, DmaState::TRANSFER, 5, 2, 4, 12, 0, 0, 0, 0, 0, 0};
	Descriptor_5D dma_5d_mm2s_suspend = {1, 0, DmaState::SUSPENDED, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	Descriptor_5D dma_5d_s2mm_wait = {1, 0, DmaState::WAIT, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
	Descriptor_5D dma_5d_s2mm_transfer = {2, 100, DmaState::TRANSFER, 5, 2, 4, 12, 0, 0, 0, 0, 0, 0};
	Descriptor_5D dma_5d_s2mm_suspend = {2, 0, DmaState::SUSPENDED, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	dma_5d_mm2s.loadProgram({dma_5d_mm2s_transfer,
							 dma_5d_mm2s_suspend});

	dma_5d_s2mm.loadProgram({dma_5d_s2mm_wait,
							 dma_5d_s2mm_transfer,
							 dma_5d_s2mm_suspend});

	std::cout << "@" << sc_time_stamp() << " Load Pulse " << std::endl;

	clk = 1;
	sc_start(0.5, SC_NS);
	clk = 0;
	sc_start(0.5, SC_NS);

	enable = 1;

	std::cout << "@" << sc_time_stamp() << " Running Transfer " << std::endl;

	for (unsigned int i = 0; i < 51; i++)
	{
		clk = 1;
		sc_start(0.5, SC_NS);
		clk = 0;
		sc_start(0.5, SC_NS);
		// std::cout << "@" << sc_time_stamp() << " Stream Out " << stream << std::endl;
	}

	std::cout << "@" << sc_time_stamp() << " Validating Transfer " << std::endl;

	for (unsigned int j = 0; j < 100; j += 10)
	{
		for (unsigned int i = j; i < j + 10; i += 2)
		{

			if (ram[i] != ram[i + 100])
			{
				std::cout << "@" << sc_time_stamp() << " validation failed :(" << std::endl;
				std::cout << "@" << sc_time_stamp() << " ram [" << i << "]: " << ram[i] << " != "
						  << "ram[" << i + 100 << "]: " << ram[i + 100] << std::endl;

				return false;
			}
		}
		j = j + 10;
	}

	std::cout << "@" << sc_time_stamp() << " Transfer Complete " << std::endl;
	std::cout << "@" << sc_time_stamp() << " 2D Strided Transfer Validation Succuss " << std::endl;

	return true;
}

void printBreak()
{
	for (unsigned int i = 0; i < 20; i++)
	{
		std::cout << " - ";
	}
	std::cout << std::endl;
}

int sc_main(int argc, char *argv[])
{

	printBreak();
	// std::cout << "Starting 2D Strided Test " << std::endl;
	// if(!run2DStridedTransferTest())
	// {
	// 	std::cout << " 2D Strided Transfer Validation FAIL :( " << std::endl;
	// 	return -1;
	// }

	if (!runIm2ColTest())
	{
		std::cout << " Im2Col Transfer Validation FAIL :( " << std::endl;
		return -1;
	}

	std::cout << "TEST BENCH SUCCESS " << std::endl
			  << std::endl;

	std::cout << "       aOOOOOOOOOOa" << std::endl;
	std::cout << "     aOOOOOOOOOOOOOOa" << std::endl;
	std::cout << "   aOO    OOOOOO    OOa" << std::endl;
	std::cout << "  aOOOOOOOOOOOOOOOOOOO0a" << std::endl;
	std::cout << " aOOOOO   OOOOOO   OOOOOa" << std::endl;
	std::cout << "aOOOOO     OOOO     OOOOOa" << std::endl;
	std::cout << "aOOOOOOOOOOOOOOOOOOOOOOOOa" << std::endl;
	std::cout << "aOOOOOOOOOOOOOOOOOOOOOOOOa" << std::endl;
	std::cout << "aOOOOO   OOOOOOOO   OOOOOa" << std::endl;
	std::cout << " aOOOOO    OOOO    OOOOOa" << std::endl;
	std::cout << "  aOOOOO          OOOOOa" << std::endl;
	std::cout << "   aOOOOOOOOOOOOOOOOOOa" << std::endl;
	std::cout << "     aOOOOOOOOOOOOOOa" << std::endl;
	std::cout << "       aOOOOOOOOOOa" << std::endl;

	return 0;
}
