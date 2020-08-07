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
		control.set_reset(false);
		cout << "Instantiated AddressGenerator TB with name " << this->name() << endl;
	}
	bool validate_reset()
	{
		cout << "Validating Reset" << endl;
		control.set_reset(false);
		sc_start(1, SC_NS);
		control.set_reset(true);
		sc_start(1, SC_NS);
		if (!(dut.descriptors.at(0) == default_descriptor))
		{
			cout << "dut.descriptors.at(0) == default_descriptor FAILED!" << endl;
			return false;
		}
		if (!(dut.execute_index == 0))
		{
			cout << "dut.execute_index == 0 FAILED!" << endl;
			return false;
		}
		if (!(dut.current_ram_index == 0))
		{
			cout << "dut.current_ram_index == 0 FAILED!" << endl;
			return false;
		}
		if (!(dut.x_count_remaining == 0))
		{
			cout << "dut.x_count_remaining == 0 FAILED!" << endl;
			return false;
		}
		if (!(dut.y_count_remaining == 0))
		{
			cout << "dut.y_count_remaining == 0 FAILED!" << endl;
			return false;
		}
		control.set_reset(false);
		sc_start(1, SC_NS);
		cout << "validate_reset Success" << endl;
		return true;
	}

	bool validate_loadprogram_and_suspended_state()
	{
		cout << "Validating validate_loadprogram_and_suspended_state" << endl;

		Descriptor_2D suspend_descriptor(0,0,DescriptorState::SUSPENDED, 0, 0, 0, 0);
		vector<Descriptor_2D> temp_program;
		temp_program.push_back(suspend_descriptor);
		dut.loadProgram(temp_program);
		control.set_enable(true);
		sc_start(10, SC_NS);

		if(!(dut.descriptors.at(0) == suspend_descriptor))
		{
			cout << "dut.descriptors.at(0) == suspend_descriptor FAILED!" << endl;
			return false;
		}
		if(!(dut.currentDescriptor() == suspend_descriptor))
		{
			cout << "dut.currentDescriptor() == suspend_descriptor FAILED!" << endl;
			return false;
		}
		if(!(dut.execute_index == 0))
		{
			cout << "dut.execute_index == 0 FAILED!" << endl;
			return false;
		}
		if(!(dut.x_count_remaining == 0))
		{
			cout << "dut.x_count_remaining == 0 FAILED!" << endl;
			return false;
		}
		if(!(dut.y_count_remaining == 0))
		{
			cout << "dut.y_count_remaining == 0 FAILED!" << endl;
			return false;
		}
		cout << "validate_loadprogram_and_suspended_state Success" << endl;
		return true;
	}

	bool validate_wait()
	{
		cout << "Validating validate_wait" << endl;
		
		control.set_enable(false);
		control.set_reset(true);
		sc_start(1, SC_NS);
		control.set_reset(false);
		sc_start(1, SC_NS);
		Descriptor_2D wait_descriptor(1,0,DescriptorState::WAIT, 5, 1, 5, 1);
		vector<Descriptor_2D> temp_program;
		temp_program.push_back(wait_descriptor);
		dut.loadProgram(temp_program);
		control.set_enable(true);
		sc_start(1, SC_NS);
		for (unsigned int i = 0; i < 10; i++)
		{
			sc_start(1, SC_NS);
		}
		
		// sc_start(10, SC_NS);
		if(!(dut.current_ram_index == 10))
		{
			cout << "dut.current_ram_index == 10 FAILED!" << endl;
			return -1;
		}
		if(!(dut.channel->enabled() == false))
		{
			cout << "dut.channel->enabled() == false FAILED!" << endl;
			return -1;
		}
		cout << "validate_wait SUCCESS" << endl;
		return true;
	}

	int run_tb()
	{
		if (!validate_reset())
		{
			cout << "validate_reset() FAILED!" << endl;
			return -1;
		}

		if(!(validate_loadprogram_and_suspended_state()))
		{
			cout << "validate_loadprogram_and_suspended_state() FAILED!" << endl;
			return -1;
		}

		if(!(validate_wait()))
		{
			cout << "validate_wait() FAILED!" << endl;
			return -1;
		}

		cout << "TEST BENCH SUCCESS " << endl << endl;
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