#include <systemc.h>
#include "Right.cpp"
#include <assert.h>

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
	sc_signal<float> stream_in("stream_in");	
  sc_signal<float> stream_out("stream_out");
  sc_signal<float> ground("gnd");
	
  float ram0[2*IMAGE_SIZE];
  float ram1[2*IMAGE_SIZE];
  float ram2[2*IMAGE_SIZE];

  float expected_output[64] = {15678, 15813, 15948, 16083, 16218, 16353, 16488, 16623, 17028, 17163, 17298, 17433, 17568, 17703, 17838, 17973, 18378, 18513, 18648, 18783, 18918, 19053, 19188, 19323, 19728, 19863, 19998, 20133, 20268, 20403, 20538, 20673, 21078, 21213, 21348, 21483, 21618, 21753, 21888, 22023, 22428, 22563, 22698, 22833, 22968, 23103, 23238, 23373, 23778, 23913, 24048, 24183, 24318, 24453, 24588, 24723, 25128, 25263, 25398, 25533, 25668, 25803, 25938, 26073};
  
	for (int i = 0; i < IMAGE_SIZE ; i++)
  {
    ram0[i] = i+1;
    ram1[i] = i+1+100;
    ram2[i] = i+1+200;    
  }

  RIGHT cloud("compute", clk, reset, enable, ram0, ram1, ram2, stream_out) ;

  //branch0 descriptors
  Descriptor desc_branch0_group0_transfer = {2, 0, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch0_group0_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};

  Descriptor desc_branch0_group1_timed_wait = {1, 0, DmaState::WAIT, 5, 1}; // 2
  Descriptor desc_branch0_group1_transfer = {2, 10, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch0_group1_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};

  Descriptor desc_branch0_group2_timed_wait = {1, 0, DmaState::WAIT, 10, 1}; // 4
  Descriptor desc_branch0_group2_transfer = {2, 20, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch0_group2_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};

  //branch1 descriptors
  Descriptor desc_branch1_group0_timed_wait = {1, 0, DmaState::WAIT, 17, 1}; // 8
  Descriptor desc_branch1_group0_transfer = {2, 0, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch1_group0_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};


  Descriptor desc_branch1_group1_timed_wait = {1, 0, DmaState::WAIT, 22, 1};
  Descriptor desc_branch1_group1_transfer = {2, 10, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch1_group1_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};


  Descriptor desc_branch1_group2_timed_wait = {1, 0, DmaState::WAIT, 27, 1};
  Descriptor desc_branch1_group2_transfer = {2, 20, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch1_group2_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};

  //branch2 descriptors
  Descriptor desc_branch2_group0_timed_wait = {1, 0, DmaState::WAIT, 34, 1};
  Descriptor desc_branch2_group0_transfer = {2, 0, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch2_group0_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};


  Descriptor desc_branch2_group1_timed_wait = {1, 0, DmaState::WAIT, 39, 1};
  Descriptor desc_branch2_group1_transfer = {2, 10, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch2_group1_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};


  Descriptor desc_branch2_group2_timed_wait = {1, 0, DmaState::WAIT, 44, 1};
  Descriptor desc_branch2_group2_transfer = {2, 20, DmaState::TRANSFER, IMAGE_SIZE, 1};
  Descriptor desc_branch2_group2_suspend = {2, 0, DmaState::SUSPENDED, 0, 1};

  // File to trace down signals  
  sc_trace_file *wf = sc_create_vcd_trace_file("compute side");
  sc_trace(wf, enable, "enable");
  sc_trace(wf, reset, "reset");
  sc_trace(wf, clk, "clk");
  sc_trace(wf, stream_out, "output");

  // Start simulation 
  enable = 0;
  reset = 1;
  
  sc_start(0, SC_NS);
  
  reset = 0;

  sc_start(1, SC_NS);
  
  cloud.branch0.group0.dma_mm2s.loadProgram({desc_branch0_group0_transfer, desc_branch0_group0_suspend});
  cloud.branch0.group1.dma_mm2s.loadProgram({desc_branch0_group1_timed_wait, desc_branch0_group1_transfer, desc_branch0_group1_suspend});
  cloud.branch0.group2.dma_mm2s.loadProgram({desc_branch0_group2_timed_wait, desc_branch0_group2_transfer, desc_branch0_group2_suspend});

  cloud.branch1.group0.dma_mm2s.loadProgram({desc_branch1_group0_timed_wait, desc_branch1_group0_transfer, desc_branch1_group0_suspend});
  cloud.branch1.group1.dma_mm2s.loadProgram({desc_branch1_group1_timed_wait, desc_branch1_group1_transfer, desc_branch1_group1_suspend});
  cloud.branch1.group2.dma_mm2s.loadProgram({desc_branch1_group2_timed_wait, desc_branch1_group2_transfer, desc_branch1_group2_suspend});

  cloud.branch2.group0.dma_mm2s.loadProgram({desc_branch2_group0_timed_wait, desc_branch2_group0_transfer, desc_branch2_group0_suspend});
  cloud.branch2.group1.dma_mm2s.loadProgram({desc_branch2_group1_timed_wait, desc_branch2_group1_transfer, desc_branch2_group1_suspend});
  cloud.branch2.group2.dma_mm2s.loadProgram({desc_branch2_group2_timed_wait, desc_branch2_group2_transfer, desc_branch2_group2_suspend});

  cloud.branch0.group0.loadWeights({1,2,3});
  cloud.branch0.group1.loadWeights({4,5,6});
  cloud.branch0.group2.loadWeights({7,8,9});

  cloud.branch1.group0.loadWeights({1,2,3});
  cloud.branch1.group1.loadWeights({4,5,6});
  cloud.branch1.group2.loadWeights({7,8,9});

  cloud.branch2.group0.loadWeights({1,2,3});
  cloud.branch2.group1.loadWeights({4,5,6});
  cloud.branch2.group2.loadWeights({7,8,9});

  stream_in = 0;

  std::cout << "@" << sc_time_stamp() << " Load Pulse " << std::endl;

  clk = 0;
  sc_start(0.5, SC_NS);
  clk = 1;
  sc_start(0.5, SC_NS);
  enable = 1;

  bool startValidation = false;
  int expected_output_index = 0;
  int validCounter = 8;
  int invalidCounter = 1;
  for(int k = 0; expected_output_index < 64; ++k) {
    
    if(!startValidation && expected_output[0] == stream_out)
    {
      startValidation = true;
    }

    if(startValidation)
    {
      if(validCounter > 0)
      {
        std::cout << "@" << sc_time_stamp() << " stream_out: " << stream_out \
        << " expected_output: " << expected_output[expected_output_index];

        if(expected_output[expected_output_index] == stream_out)
        {
          std::cout << " ....assertion succuess!!" << std::endl;
          expected_output_index++;
        }
        else
        {
          std::cout << " ....assertion failure, terminating....!!" << std::endl;
          break;
        }
        validCounter--;
      }
      else if(invalidCounter > 0)
      {
        std::cout << "@" << sc_time_stamp() << " stream_out: " << stream_out \
        << " ....ignoring invalid output " << std::endl;
        invalidCounter--;
      }
      else
      {
        std::cout << "@" << sc_time_stamp() << " stream_out: " << stream_out \
        << " ....ignoring invalid output " << std::endl;
        validCounter = 8;
        invalidCounter = 1;
      }
    }

    clk = 0;
    sc_start(0.5, SC_NS);
    clk = 1;
    sc_start(0.5, SC_NS);
    
  }

  if(expected_output_index == 64)
  {
    std::cout << "TEST BENCH SUCCESS " << std::endl;


    std::cout << "       a$$$$$$$$$$a" << std::endl;
    std::cout << "     a$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "   a$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "  a$$$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << " a$$$$$   $$$$$$   $$$$$a" << std::endl;
    std::cout << "a$$$$$     $$$$     $$$$$a" << std::endl;
    std::cout << "a$$$$$$$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "a$$$$$$$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "a$$$$$$$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << " a$$$$$$          $$$$$$a" << std::endl;
    std::cout << "  a$$$$$$        $$$$$$a" << std::endl;
    std::cout << "   a$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "     a$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "       a$$$$$$$$$$a" << std::endl;

  }
  else
  {
    std::cout << "TEST BENCH FAILURE " << std::endl;

    std::cout << "       a$$$$$$$$$$a" << std::endl;
    std::cout << "     a$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "   a$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "  a$$$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << " a$$$$$   $$$$$$   $$$$$a" << std::endl;
    std::cout << "a$$$$$     $$$$     $$$$$a" << std::endl;
    std::cout << "a$$$$$$$ $$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "a$$$$$$$ $$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "a$$$$$$$$$$$$$$$$$$$$$$$$a" << std::endl;
    std::cout << " a$$$$$$          $$$$$$a" << std::endl;
    std::cout << "  a$$$$  $$$$$$$$  $$$$a" << std::endl;
    std::cout << "   a$$ $$$$$$$$$$$$ $$a" << std::endl;
    std::cout << "     a$$$$$$$$$$$$$$a" << std::endl;
    std::cout << "       a$$$$$$$$$$a" << std::endl;
  }

  return 0;

}
