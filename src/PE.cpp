//-----------------------------------------------------
// This is my second Systemc Example
// Design Name : first_counter
// File Name : first_counter.cpp
// Function : This is a 4 bit up-counter with
// Synchronous active high reset and
// with active high enable signal
//-----------------------------------------------------

#ifndef PE_CPP // Note include guards, this is a quick and dirty way to include components
#define PE_CPP 

#include "systemc.h"
#include "map"
#include "vector"

//-----------------------------------------------------
// Design Name : PE
// File Name   : PE.cpp
// Function    : Nothing.... used with tutorial 
// Coder       : Aly Sultan 
//-----------------------------------------------------
#include "systemc.h"
#include <iostream>

using std::cout;
using std::endl;

/**
 * @brief Construct a new sc module object (This is basically a macro )
 * 
 * Inputs: clk and asynchronous input active high
 * 
 */
SC_MODULE (PE) 
{
  //-----------Input Ports---------------
  sc_in <bool> clk, reset;


  /**
   * @brief Methods defined in sensitivty list that will be called when a
   * positive edge is detected on clk on reset is set to high. Please note that
   * since reset is evaluated before anything, a reset will always take priority
   * over any other signal change occuring with reset. (Ex a rising edge on a
   * clk combined with a high signal on reset). sc_time_stamp() will print
   * current global simulation time. 
   * 
   */
  void update () 
  {
    if (reset.read()) 
    { 
      cout << "@ " << sc_time_stamp() << " Module has been reset" << endl;
    } 
    else 
    {
      cout << "@ " << sc_time_stamp() << " Module has recieved a rising edge on clk signal" << endl;
    }
  }


  /**
   * @brief Construct a new SC_CTOR object of type PE. Please note that a sc
   * ctor object isn't something ultra special, it's just a struct and sc_ctor a macro
   * that expands into a struct constructor. There are different types of
   * systemc object constructors. They will be defined in the tutorial. Since
   * all systemc objects are just structs we can take advantage of inheritance
   * and object composition to simulate bigger components. This point will be
   * relevant beyond the tutorial. 
   * 
   * 
   * Note: I have commented out the original SC_CTOR constructor to 1) show
   * that it is infact just a macro and to 2) to show that you can pass other
   * things to the system c object constructor. In this case we can pass signals
   * directly to the constructor setting them later. This simplifies systemc
   * object declaration a lot and reduces the clutter of signals you would
   * normally have in languages like VHDL and Verilog. To be fair SystemC has the
   * option of instantiating module signals by position: 
   * http://www.asic-world.com/systemc/modules3.html#Module_Instanciating
   * But the problem with this approach is that if you forget to list a signal
   * to connect the runtime will catch it instead of the compiler at compile
   * time. This methods I have is much easier. 
   * 
   */
  typedef PE SC_CURRENT_USER_MODULE; PE( ::sc_core::sc_module_name name, sc_signal<bool>* _clk, sc_signal<bool>* _reset)
  {
    

      /**
       * @brief A lot of magic happens here but the tldr is that SC_METHOD
       * hooks this module up to systemC's internal runtime system so that when a
       * change is detected in clk, PE's member function "update" is called. The
       * SystemC library can be considerd as a mini OS with a scheduler that
       * "mimics" the passage of time and updates modules states accordingly.
       * This is similair to Modelsim for VHDL and Verilog. For our purposes we
       * don't have to philosophize too much about what the hell systemC's
       * doing, but bottom line, every module has an entry point that gets
       * called when signals change. This is where you define it.  
       * 
       * 
       */      
      SC_METHOD (update);

      /**
       * @brief These few lines establish what signal will cause the update
       * function to get called. Since no particular edge was defined for reset
       * it is assumed level trigger.
       * 
       */
      sensitive << clk.pos(); 
      sensitive << reset;
      
      /**
       * @brief external signals are hooked up to internal module ports here.
       * a weird way of doing it because it almost looks like a constructor but
       * there's an operator overload in all port types that allows you to do it
       * this way. 
       * 
       */
      this->clk(*_clk);
      this->reset(*_reset);

      cout << "Module : " << name << " has been instantiated " << endl;

  }

  // SC_CTOR(PE)
  // {
  //     SC_METHOD (update);
  //     sensitive << clk.pos(); 
  //     sensitive << reset;
  // }
};
#endif
