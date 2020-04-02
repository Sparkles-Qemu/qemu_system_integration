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
// Coder       : Jacob Londa
//-----------------------------------------------------

using std::cout;
using std::endl;

enum DmaDirection
{
  MM2S,
  S2MM
};

struct DmaConfig
{
  bool enable;           // enable DMA through config field
  sc_uint<32> wait;      // number of transfers to wait before proceeding
};

struct Descriptor
{
  Descriptor* next;      // pointer to next descriptor
  sc_uint<32> start;     // start index in ram array
  DmaConfig* config;     // pointer to config field
  sc_uint<32> x_count;   // number of floats to transfer
  sc_uint<32> x_modify;  // number of floats between each transfer
};

// DMA module definition
struct DMA : public sc_module
{
  // Control Signals
  sc_in<bool> clk, reset, enable;

  // Memory and Stream Ports
  sc_inout<float*> ram;
  sc_inout<float> stream;

  // Internal Data
  Descriptor* descriptor;
  sc_uint<32> execute_index;
  DmaDirection direction;
  sc_uint<32> current_ram_index;
  sc_uint<32> x_count_remaining;

  // Called on rising edge of clk or high level reset
  void update () 
  {
    if (reset.read()) 
    {
      execute_index = 0;
      current_ram_index = descriptor->start;
      x_count_remaining = descriptor->x_count;
      cout << "@ " << sc_time_stamp() << " Module has been reset" << endl;
    } 
    else if (enable.read() && descriptor->config->enable && (x_count_remaining > 0))  // TODO(Jacob): should x_count_remaining check be here?
    {  // TODO(Jacob): add wait count check somewhere here
      if (direction == MM2S)
        stream = *(ram + current_ram_index);        // Memory to Stream
      else
        *(ram + current_ram_index) = stream;        // Stream to Memory
      
      x_count_remaining--;

      if (x_count_remaining == 0)
        execute_index++;                            // decsriptor is finished, all transfers complete  TODO(Jacob): load next descriptor automatically here?
      else
        current_ram_index += descriptor->x_modify;  // descriptor is still active, update ram index
    }
  }

  // Constructor
  DMA(sc_module_name name, DmaDirection _direction, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, 
          const sc_signal<bool>& _enable, sc_signal<float*>& _ram, sc_signal<float>& _stream)
  {
      SC_METHOD(update);
        sensitive << reset;
        sensitive << clk.pos();
      
      // connect signals
      this->direction = _direction;
      this->clk(_clk);
      this->clk(_reset);
      this->clk(_enable);
      this->ram(_ram);
      this->stream(_stream);

      cout << "Module : " << name << " has been instantiated " << endl;
  }

  SC_HAS_PROCESS(DMA);
};
#endif
