#ifndef LEFT_CPP
#define LEFT_CPP 

#include <systemc.h>
#include <string>
#include <DMA.cpp>

SC_MODULE(LEFT)
{
	// Stream from MM2S to S2MMs
	sc_signal<float,SC_MANY_WRITERS> stream_bus;

	// Memory To Stream
	DMA dma_mm2s;

	// Stream to Memories
	DMA dma_s2mm1, dma_s2mm2, dma_s2mm3;

	// Member Functions
	void print_info(void);
	
	// Constructor with init list of components
	LEFT(sc_module_name name, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable, float* _ram_source, float* _ram_dest1, float* _ram_dest2, float* _ram_dest3):
dma_mm2s("internal_s2mm", DmaDirection::MM2S, _clk, _reset, _enable, _ram_source, stream_bus),
dma_s2mm1("internal_mm2s1", DmaDirection::S2MM, _clk, _reset, _enable, _ram_dest1, stream_bus),
dma_s2mm2("internal_mm2s2", DmaDirection::S2MM, _clk, _reset, _enable, _ram_dest2, stream_bus),
dma_s2mm3("internal_mm2s3", DmaDirection::S2MM, _clk, _reset, _enable, _ram_dest3, stream_bus)
	{
		std::cout << "Module: " << name << " has been instantiated" << std::endl;
	}
	
	SC_HAS_PROCESS(LEFT);

};

#endif // LEFT_CPP
