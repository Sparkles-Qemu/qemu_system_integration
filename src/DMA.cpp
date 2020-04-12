#ifndef PE_CPP // Note include guards, this is a quick and dirty way to include components
#define PE_CPP 

#include "systemc.h"
#include "map"
#include "vector"
#include <string>
#include <iostream>

//-----------------------------------------------------
// Design Name : DMA
// File Name   : DMA.cpp
// Function    : Direct Memory Access
// Coder       : Jacob Londa and Owen Beringer
//-----------------------------------------------------

enum class DmaDirection
{
  MM2S,
  S2MM
};

enum class DmaState
{
  SUSPENDED,             // do nothing indefinitely
  TRANSFER,              // transfer data
  WAIT                   // do nothing for certain number of cycles
};

struct Descriptor
{
  sc_uint<32> next;      // index of next descriptor
  sc_uint<32> start;     // start index in ram array
  DmaState state;        // state of dma
  sc_uint<32> x_count;   // number of floats to transfer/wait
  sc_uint<32> x_modify;  // number of floats between each transfer/wait
};

// DMA module definition for MM2S and S2MM
struct DMA : public sc_module
{
  // Control Signals
  sc_in<bool> clk, reset, enable;

  // Memory and Stream
  float* ram;
  sc_inout<float> stream;

  // Internal Data
  std::vector<Descriptor> descriptors;
  sc_uint<32> execute_index;
  DmaDirection direction;
  sc_uint<32> current_ram_index;
  sc_uint<32> x_count_remaining;

  // Prints descriptor list, useful for debugging
  void print_descriptors()
  {
    for (int i = 0; i < descriptors.size(); i++)
    {
      std::cout << "Descriptor " << i << std::endl;
      std::cout << "next: " << descriptors[i].next << std::endl;
      std::cout << "start: " << descriptors[i].start << std::endl;
      std::cout << "state: " << (int)descriptors[i].state << std::endl;
      std::cout << "x_count: " << descriptors[i].x_count << std::endl;
      std::cout << "x_modify: " << descriptors[i].x_modify << std::endl << std::endl;
    }
  }

  // Called on rising edge of clk or high level reset
  void update() 
  {
   if (reset.read()) 
    {
      // assume at least one descriptor is in dma at all times
      execute_index = 0;
      current_ram_index = descriptors[execute_index].start;
      x_count_remaining = descriptors[execute_index].x_count;
      descriptors[execute_index].state == DmaState::SUSPENDED;  // slightly cheating here, but does what we want
      std::cout << "@ " << sc_time_stamp() << " Module has been reset" << std::endl;
    } 
    else if (enable.read() && (descriptors[execute_index].state != DmaState::SUSPENDED))
    {
      if (descriptors[execute_index].state == DmaState::TRANSFER)
      {
        std::cout << "@ " << sc_time_stamp() << " d" << execute_index << " Transfering data" << std::endl;
        if (direction == DmaDirection::MM2S)
          stream.write(*(ram + current_ram_index));  // Memory to Stream
        else
          *(ram + current_ram_index) = stream.read();  // Stream to Memory

        // update ram index
        current_ram_index += descriptors[execute_index].x_modify;
      }
      else  // just for debugging, can be removed
        std::cout << "@ " << sc_time_stamp() << " d" << execute_index << " Waiting..." << std::endl;
      
      x_count_remaining--;

      if (x_count_remaining == 0)  // descriptor is finished, load next descriptor
      {
        execute_index = descriptors[execute_index].next;
        current_ram_index = descriptors[execute_index].start;
        x_count_remaining = descriptors[execute_index].x_count;
      }
    }
  }

  // Constructor
  DMA(sc_module_name name, DmaDirection _direction, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset,const sc_signal<bool>& _enable, float* _ram, sc_signal<float,SC_MANY_WRITERS>& _stream)
  {
      SC_METHOD(update);
        sensitive << reset;
        sensitive << clk.pos();
      
      // connect signals
      this->direction = _direction;
      this->clk(_clk);
      this->reset(_reset);
      this->enable(_enable);
      this->ram = _ram;
      this->stream(_stream);

      std::cout << "Module : " << name << " has been instantiated " << std::endl;
  }

  SC_HAS_PROCESS(DMA);
};

// DMA module definition for MM2MM
struct DMA_MM2MM : public sc_module
{
	// Control Signals
	sc_in<bool> clk, reset, enable;

	// stream interconnect
	sc_signal<float,SC_MANY_WRITERS> stream;

	// DMA devices
	DMA *mm2s, *s2mm;

	// Constructor
	DMA_MM2MM(sc_module_name name, DMA& mm2s_in, DMA& s2mm_in, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable)
	{
		// Point to DMA devices
		mm2s = &mm2s_in;
		s2mm = &s2mm_in;

		// connect signals
		this->clk(_clk);
		this->reset(_reset);
		this->enable(_enable);

		std::cout << "Module : " << name << " has been instantiated" << std::endl;
	}

	SC_HAS_PROCESS(DMA_MM2MM);
};


#endif
