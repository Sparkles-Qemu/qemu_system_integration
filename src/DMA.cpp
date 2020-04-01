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
  bool enable;
  sc_uint<32> wait;
};

struct Descriptor
{
  Descriptor* next;
  float* start;
  DmaConfig config;
  sc_uint<32> x_count;
  sc_uint<32> x_modify;
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
  Descriptor descriptor;
  sc_uint<32> execute_index;
  DmaDirection direction;

  // Called on rising edge of clk or high level reset
  void update () 
  {
    if (reset.read()) 
    { 
      cout << "@ " << sc_time_stamp() << " Module has been reset" << endl;
    } 
    else if (enable.read())
    {
      cout << "@ " << sc_time_stamp() << " Module has recieved a rising edge on clk signal" << endl;
    }
  }

  // Constructor
  DMA(sc_module_name name, DmaDirection _direction, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, 
          const sc_signal<bool>& _enable, sc_signal<float*>& _ram, sc_signal<float>& _stream)
  {
      SC_METHOD(update);
        sensitive << reset;
        sensitive << clk.pos();
      
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
