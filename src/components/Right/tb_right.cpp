#include <systemc.h>
#include "Right.cpp"

#define IMAGE_WIDTH 10
#define IMAGE_HEIGHT 10
#define IMAGE_SIZE IMAGE_WIDTH*IMAGE_HEIGHT
#define WAIT_CYCLES 3

void print_ram(float* ram, int size)

{
	for (int i = 0; i < size; i++)
		std::cout << *(ram + i) << " ";
	std::cout << "\n\n";
}

int sc_main(int argc, char *argv[])
{
	sc_signal<bool> clk("clk"); 
	sc_signal<bool> reset("reset"); 
	sc_signal<bool> enable("enable"); 
	sc_signal<float> stream("stream");	
  sc_signal<float> stream_out("stream_out");
	
	// RAM representing external Memory
	float ram[IMAGE_SIZE];

  // Expected values after every clock cycle
  float expectedValue[] = {0, 0, 0, 0, 0, 0, 0, 198, 726, 771, 816, 861, 906, 951, 996, 1041, 1086, 1131, 1176, 1221, 1266, 1311, 1356, 1401, 1446, 1491, 1536, 1581, 1626, 1671, 1716, 1761, 1806, 1851, 1896, 1941, 1986, 2031, 2076, 2121, 2166, 2211, 2256, 2301, 2346, 2391, 2436, 2481, 2526, 2571, 2616, 2661, 2706, 2751, 2796, 2841, 2886, 2931, 2976, 3021, 3066, 3111, 3156, 3201, 3246, 3291, 3336, 3381, 3426, 3471, 3516, 3561, 3606, 3651, 3696, 3741, 3786, 3831, 3876, 3921, 3966, 4011, 4056, 4101, 4146, 4191};
	// Fill source ram
	for (int i = 0; i < IMAGE_SIZE ; i++)
		ram[i] = i;

	// instantiate left side of processor
  RIGHT right("compute", clk, reset, enable, ram, ram, ram, stream_out);

  Descriptor desc_branch00_transfer = {0, 0, DmaState::TRANSFER, IMAGE_SIZE, 1};
  right.branch0.group0.dma_mm2s.descriptors.push_back(desc_branch00_transfer);
  right.branch0.group1.dma_mm2s.descriptors.push_back(desc_branch00_transfer);
  right.branch0.group2.dma_mm2s.descriptors.push_back(desc_branch00_transfer);
  
   
  // Start simulation 
  sc_start(0, SC_NS);

  // File to trace down signals  
  sc_trace_file *wf = sc_create_vcd_trace_file("compute side");
  sc_trace(wf, enable, "enable");
  sc_trace(wf, reset, "reset");
  sc_trace(wf, clk, "clk");
  sc_trace(wf, stream_out, "output");

  // Start test bench 
  enable.write(1);

  for(int k = 0; k < IMAGE_SIZE; ++k) {

    //Start simulation here
    clk.write(0);
    sc_start(1, SC_NS);
    clk.write(1);
    sc_start(1, SC_NS);
    
  }

  return 0;

}
