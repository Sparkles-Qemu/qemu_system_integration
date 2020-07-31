#include <systemc.h>
#include "AddressGenerator.cpp"
// #define DEBUG
using std::cout;
using std::endl;

template <typename DataType>
struct AddressGenerator_TB : public sc_module
{

	const unsigned int mem_channel_width = 4;

	sc_trace_file *tf;
	GlobalControlChannel control;

	AddressGenerator<DataType> dut;
	MemoryChannel<DataType> mem_channel;
	AddressGenerator_TB(sc_module_name name) : sc_module(name),
											   tf(sc_create_vcd_trace_file("ProgTrace")),
											   control("global_control_channel", sc_time(1, SC_NS), tf),
											   dut("dut", control, tf),
											   mem_channel("mem_channel", mem_channel_width, tf)
	{
		tf->set_time_unit(1, SC_PS);
		dut.channel(mem_channel);
		cout << "Instantiated AddressGenerator TB with name " << this->name() << endl;
	}
	bool validate_reset()
	{
		control.set_reset(true);
		sc_start(1, SC_NS);
		if (!(dut.descriptors.at(0) == default_descriptor))
		{
			cout << "dut.descriptors.at(0) == default_descriptor FAILED!" << endl;
			return false;
		}
		if (!(dut.execute_index.read() == 0))
		{
			cout << "dut.execute_index.read() == 0 FAILED!" << endl;
			return false;
		}
		if (!(dut.current_ram_index.read() == 0))
		{
			cout << "dut.current_ram_index.read() == 0 FAILED!" << endl;
			return false;
		}
		if (!(dut.x_count_remaining.read() == 0))
		{
			cout << "dut.x_count_remaining.read() == 0 FAILED!" << endl;
			return false;
		}
		if (!(dut.y_count_remaining.read() == 0))
		{
			cout << "dut.y_count_remaining.read() == 0 FAILED!" << endl;
			return false;
		}
		return true;
	}

	bool validate_suspended()
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
		cout << "TEST BENCH SUCCESS " << endl;
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
	AddressGenerator_TB<unsigned int> tb("AddressGenerator_tb");
	return tb.run_tb();
}