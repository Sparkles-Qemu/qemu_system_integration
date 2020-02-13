//-----------------------------------------------------
// This is my second Systemc Example
// Design Name : first_counter
// File Name : first_counter.cpp
// Function : This is a 4 bit up-counter with
// Synchronous active high reset and
// with active high enable signal
//-----------------------------------------------------

#ifndef PE_CPP
#define PE_CPP 

#include "systemc.h"
#include "map"
#include "vector"

SC_MODULE(PE)
{
    SC_CTOR(PE)
    {
        SC_METHOD(update);
        sensitive << reset;
        sensitive << clock.pos();
    } // End of Constructor

}; // End of Module counter

#endif
