#include <systemc.h>
#include "Memory.cpp"

using std::cout;
using std::endl;

unsigned int ram_size = 256;
unsigned int read_port_count = 1;
unsigned int write_port_count = 1;

sc_trace_file *tf = sc_create_vcd_trace_file("Prog_trace");

GenericControlBus control("global", tf);

Memory<float> mem("mem",
				  control,
				  ram_size,
				  read_port_count,
				  write_port_count,
				  tf);

bool validate_reset()
{
	control.reset = 0;
	control.clk = 0;
	sc_start(1, SC_NS);
	control.reset = 1;
	sc_start(1, SC_NS);
	control.reset = 0;

	for (auto val : mem.ram)
	{
		if (val != 0)
		{
			return false;
		}
	}

	return true;
}

bool validate_write()
{
	control.enable = 1;
	mem.write_bus_bundle.at(0).enable = 1;
	for (unsigned int i = 0; i < mem.ram.size(); i++)
	{
		mem.write_bus_bundle.at(0).addr = i;
		mem.write_bus_bundle.at(0).data = i+1;
		control.clk = 0;
		sc_start(0.5, SC_NS);
		control.clk = 1;
		sc_start(0.5, SC_NS);
	}
	unsigned int expected_data = 1;
	for (auto val : mem.ram)
	{
		if (val != expected_data++)
		{
			return false;
		}
	}
	return true;
}

bool validate_read()
{
	control.enable = 1;
	mem.read_bus_bundle.at(0).enable = 1;
	unsigned int expected_data = 1;
	for (unsigned int i = 0; i < mem.ram.size(); i++)
	{
		mem.read_bus_bundle.at(0).addr = i;
		control.clk = 0;
		sc_start(0.5, SC_NS);
		control.clk = 1;
		sc_start(0.5, SC_NS);
		if(mem.read_bus_bundle.at(0).data != expected_data++)
		{
			return false;
		}
	}
	return true;
}

int sc_main(int argc, char *argv[])
{
	tf->set_time_unit(1, SC_PS);

	if (!validate_reset())
	{
		cout << "Reset Failed" << endl;
		return -1;
	}
	cout << "Reset Success" << endl;

	if(!validate_write())
	{
		cout << "Write Failed" << endl;
		return -1;
	}
	cout << "Write Success" << endl;
	
	if(!validate_read())
	{
		cout << "Read Failed" << endl;
		return -1;
	}
	cout << "Read Success" << endl;
	

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
