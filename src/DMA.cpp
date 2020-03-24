#ifndef PE_CPP // Note include guards, this is a quick and dirty way to include components
#define PE_CPP 

#include "systemc.h"
#include "map"
#include "vector"
#include <iostream>

//-----------------------------------------------------
// Design Name : DMA
// File Name   : DMA.cpp
// Function    : Direct Memory Access
// Coder       : Jacob Londa
//-----------------------------------------------------

using std::cout;
using std::endl;

struct DmaConfig
{
  bool enable;
  sc_uint<32> wait;
};

struct Descriptor
{
  Descriptor* next;
  sc_uint<8>* start;
  DmaConfig config;
  sc_uint<32> x_count;
  sc_uint<32> x_modify;
};

// DMA base module definition
SC_MODULE (DMA_Base)
{
  //-----------Input Ports---------------
  sc_in<bool> clk, reset, enable;

  //-----------Internal Data---------------
  Descriptor descriptor;
  sc_uint<32> execute_index;

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
  typedef DMA_Base SC_CURRENT_USER_MODULE; DMA_Base( ::sc_core::sc_module_name name, sc_signal<bool>* _clk, sc_signal<bool>* _reset, sc_signal<bool>* _enable)
  {   
      SC_METHOD (update);
        sensitive << clk.pos() << reset;
      
      this->clk(*_clk);
      this->reset(*_reset);
      this->enable(*_enable);

      cout << "Module : " << name << " has been instantiated " << endl;
  }
};
#endif
