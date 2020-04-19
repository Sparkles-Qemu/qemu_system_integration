#include <systemc.h>
#include <iostream>
#include <string>

#define NUM_OUTPUTS 3

// MUX module
struct MUX : public sc_module
{
	sc_in<bool> clk;
	sc_in<bool> reset;
	sc_in<float> input;
	sc_in<sc_uint<2>> select;
	sc_out<float> out1, out2, out3;
	
	void update()
	{
		int i;
		int port_num = -1;

		if(reset.read())
		{
			out1.write(0);
			out2.write(0);
			out3.write(0);
			cout << "Reset Applied" << endl;
		}

		// Write To Selected Output
		else{
			// Write Selected Ouput
			switch (select.read()){
				case 0 : out1.write(input);
				case 1 : out2.write(input);
				case 2 : out3.write(input);
				// Default writes all ports if we need
				default: out1.write(input);
					 out2.write(input);
					 out3.write(input);
			}
		}
	}

	// Constructor
	MUX(sc_module_name name, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, sc_signal<sc_uint<2>>& _select, sc_signal<float,SC_MANY_WRITERS>& _input, sc_signal<float,SC_MANY_WRITERS>& _out1, sc_signal<float,SC_MANY_WRITERS>& _out2, sc_signal<float,SC_MANY_WRITERS>& _out3)
	{
		SC_METHOD(update);
		sensitive << reset;
		sensitive << input;
		
		// Connect Signals
		this->clk(_clk);
		this->reset(_reset);
		this->select(_select);
		this->input(_input);
		this->out1(_out1);
		this->out2(_out2);
		this->out3(_out3);
		
		std::cout << "Module : " << name << " has been instantiated" << std::endl;
	}
	SC_HAS_PROCESS(MUX);
};
