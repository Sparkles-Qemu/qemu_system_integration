// COMP.h
//
#include "systemc.h"
#include <string>
#include "DMA.cpp"
#include "MUX.cpp"

SC_MODULE(LEFT)
{
	//sc_in<bool> clk;
	//sc_in<bool> enable;
	//sc_in<bool> reset;

	// Memory To Stream
	//DMA dma_mm2s;

	// Stream
	sc_signal<float,SC_MANY_WRITERS> mux_in;

	// 3 Stream Outputs 
	sc_signal<float,SC_MANY_WRITERS> mux_out1;
	sc_signal<float,SC_MANY_WRITERS> mux_out2;
	sc_signal<float,SC_MANY_WRITERS> mux_out3;

	// Mux and Select
	sc_signal<sc_uint<2>> select;
	MUX mux1;

	// Stream to Memories
	DMA dma_s2mm1, dma_s2mm2, dma_s2mm3, dma_mm2s;

	// Member Functions
	void print_info(void);
	
	// Constructor with init list of components
	LEFT(sc_module_name name, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable, float* _ram_source, float* _ram_dest1, float* _ram_dest2, float* _ram_dest3):
mux1("mux1", _clk, _reset, mux_in, mux_out1, mux_out2, mux_out3),
dma_mm2s("internal_s2mm", DmaDirection::MM2S, _clk, _reset, _enable, _ram_source, mux_in),
dma_s2mm1("internal_mm2s1", DmaDirection::S2MM, _clk, _reset, _enable, _ram_dest1, mux_out1),
dma_s2mm2("internal_mm2s2", DmaDirection::S2MM, _clk, _reset, _enable, _ram_dest2, mux_out2),
dma_s2mm3("internal_mm2s3", DmaDirection::S2MM, _clk, _reset, _enable, _ram_dest3, mux_out3)
	{
		std::cout << "Module: " << name << " has been instantiated" << std::endl;
	}
	
	SC_HAS_PROCESS(LEFT);

};
