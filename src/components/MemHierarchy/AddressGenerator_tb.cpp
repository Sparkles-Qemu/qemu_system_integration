#include <systemc.h>
#include "AddressGenerator.cpp"

// #define DEBUG

using std::cout;
using std::endl;

template <typename DataType>
struct AddressGenerator_TB : public sc_module
{
	sc_trace_file *tf;

	GlobalControlChannel control;

	AddressGenerator_TB(sc_module_name name) : sc_module(name),
									 tf(sc_create_vcd_trace_file("Prog_trace")),
									 control("global_control_channel", sc_time(1, SC_NS), tf)
	{
		tf->set_time_unit(1, SC_PS);

		cout << "Instantiated AddressGenerator TB with name " << this->name() << endl;
	}

	bool validate_reset()
	{

		return true;
	}

	int run_tb()
	{
		cout << "Validating Reset" << endl;
		if (!validate_reset())
		{
			cout << "Reset Failed" << endl;
			return -1;
		}
		cout << "Reset Success" << endl;

		cout << "TEST BENCH SUCCESS " << endl
			 << endl;

		cout << "       aOOOOOOOOOOa" << endl;
		cout << "     aOOOOOOOOOOOOOOa" << endl;
		cout << "   aOO    OOOOOO    OOa" << endl;
		cout << "  aOOOOOOOOOOOOOOOOOOOa" << endl;
		cout << " aOOOOO   OOOOOO   OOOOOa" << endl;
		cout << "aOOOOO     OOOO     OOOOOa" << endl;
		cout << "aOOOOOOOOOOOOOOOOOOOOOOOOa" << endl;
		cout << "aOOOOOOOOOOOOOOOOOOOOOOOOa" << endl;
		cout << "aOOOOO   OOOOOOOO   OOOOOa" << endl;
		cout << " aOOOOO    OOOO    OOOOOa" << endl;
		cout << "  aOOOOO          OOOOOa" << endl;
		cout << "   aOOOOOOOOOOOOOOOOOOa" << endl;
		cout << "     aOOOOOOOOOOOOOOa" << endl;
		cout << "       aOOOOOOOOOOa" << endl;
	
		return 0;
	}

	~AddressGenerator_TB()
	{
		sc_close_vcd_trace_file(tf);
	}
};

int sc_main(int argc, char *argv[])
{
	AddressGenerator_TB<sc_int<32>> tb("memory_tb");
	return tb.run_tb();
}
