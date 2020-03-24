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

  // Constructor
  typedef DMA_Base SC_CURRENT_USER_MODULE; DMA_Base( ::sc_core::sc_module_name name, sc_signal<bool>* _clk, sc_signal<bool>* _reset, sc_signal<bool>* _enable)
  {
      this->clk(*_clk);
      this->reset(*_reset);
      this->enable(*_enable);

      cout << "Module : " << name << " has been instantiated " << endl;
  }
};

// Memory to Stream DMA module definition
SC_MODULE (DMA_MM2S)
{
  //-----------Input Ports---------------
  sc_in<sc_uint<8>* > ram;
  
  //-----------Output Ports--------------
  sc_out<sc_uint<8> > stream;

  // Base DMA module
  DMA_Base* base;

  // Called on rising edge of clk or high level reset
  void update () 
  {
    if (base->reset.read()) 
    { 
      cout << "@ " << sc_time_stamp() << " Module has been reset" << endl;
    } 
    else if (base->enable.read())
    {
      cout << "@ " << sc_time_stamp() << " Module has recieved a rising edge on clk signal" << endl;
    }
  }

  // Constructor
  typedef DMA_MM2S SC_CURRENT_USER_MODULE; DMA_MM2S( ::sc_core::sc_module_name name, sc_signal<sc_uint<8>* >* _ram, sc_signal<sc_uint<8> >* _stream, 
                                                        sc_signal<bool>* _clk, sc_signal<bool>* _reset, sc_signal<bool>* _enable)
  {   
      this->ram(*_ram);
      this->stream(*_stream);

      // create base dma name  
      char base_name[sizeof(name) + 5];
      strcpy(base_name, name);
      strcat(base_name, "_base");

      // create new base dma and connect signals
      this->base = new DMA_Base(base_name, _clk, _reset, _enable);

      SC_METHOD (update);
        sensitive << base->clk.pos();
        sensitive << base->reset;

      cout << "Module : " << name << " has been instantiated " << endl;
  }
};
#endif
