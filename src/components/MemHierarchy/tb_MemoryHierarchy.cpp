#include <systemc.h>
#include "Memory.cpp"

using std::cout;
using std::endl;

unsigned int ram_length = 256;
unsigned int ram_width = 4;
unsigned int read_channel_count = 1;
unsigned int write_channel_count = 1;
unsigned int channel_count = read_channel_count + write_channel_count;

sc_trace_file *tf = sc_create_vcd_trace_file("Prog_trace");

GlobalControlChannel control("global_control_channel", sc_time(1, SC_NS), tf);

MemoryChannel<float> rchannel("read_channel", ram_width, tf);
MemoryChannel<float> wchannel("write_channel", ram_width, tf);
sc_vector<sc_signal<float>> payload("payload", ram_width);

Memory<float> mem("sram",
				  control,
				  channel_count,
				  ram_length,
				  ram_width,
				  tf);

bool validate_reset()
{
	control.set_reset(true);
	control.set_enable(false);

	sc_start(1, SC_NS);
	
	control.set_reset(false);

	for (auto& row : mem.ram)
	{
		for(auto& col : row)
		{
			if (col != float(0))
			{
				return false;
			}
		}
	}

	return true;
}

bool validate_write()
{
	control.set_enable(true);
	wchannel.set_enable(true);
	wchannel.set_mode(MemoryChannelMode::WRITE);

	unsigned int val = 1;
	for (unsigned int i = 0; i < ram_length; i++)
	{
		cout << "Generating Payload " << endl;
		for(unsigned int j = 0; j < ram_width; j++)
		{
			payload[j] = val++;
		}
		sc_start(1, SC_NS);

		cout << "Writing Payload to wchannel: " << endl;
		for(auto& data : payload)
		{
			cout << data << " ";
		}
		cout << endl;

		wchannel.write_data(payload);
		wchannel.set_addr(i);

		cout << "Writing wchannel data to memory: " << endl;
		sc_start(1, SC_NS);
	}

	cout << "validating ... ";
	unsigned int expected_data = 1;
	for (const auto& row : mem.ram)
	{
		for(const auto& col : row)
		{
			if (col != float(expected_data++))
			{
				return false;
			}
		}
	}
	cout << " success!" << endl;
	val = 1;

	cout << " writing to specific data elements ... " << endl;
	for (unsigned int i = 0; i < ram_length; i++)
	{
		wchannel.write_data_element(val, 0);
		wchannel.write_data_element(0, 1);
		wchannel.write_data_element(0, 2);
		wchannel.write_data_element(0, 3);
		wchannel.set_addr(i);

		sc_start(1, SC_NS);
		val+=4;
	}

	cout << "validating ... ";
	expected_data = 1;
	for (const auto& row : mem.ram)
	{
		if (row[0] != float(expected_data))
		{
			return false;
		}
		expected_data+=4;
	}
	return true;
}

bool validate_read()
{

	control.set_reset(true);
	sc_start(1, SC_NS);
	control.set_reset(false);
	
	if (!validate_reset())
	{
		cout << "Reset Failed" << endl;
		return -1;
	}

	control.set_enable(true);
	wchannel.set_enable(true);
	wchannel.set_mode(MemoryChannelMode::WRITE);
	rchannel.set_enable(true);
	rchannel.set_mode(MemoryChannelMode::READ);
	rchannel.set_addr(0);

	unsigned int val = 1;
	for (unsigned int i = 0; i < ram_length; i++)
	{
		for(unsigned int j = 0; j < ram_width; j++)
		{
			payload[j] = val++;
		}
		sc_start(1, SC_NS);

		wchannel.write_data(payload);
		wchannel.set_addr(i);

		sc_start(1, SC_NS);
	}

	wchannel.set_enable(false);
	unsigned int expected_data = 1;
	for(unsigned int i = 0; i<ram_length; i++)
	{
		rchannel.set_addr(i);
		sc_start(1, SC_NS);

		const sc_vector<sc_signal<float>>& payload = rchannel.read_data();
		for(unsigned int j = 0; j<ram_width; j++)
		{
			if(payload[j] != float(expected_data++))
			{
				return false;
			}
		}
	}

	return true;
}

int sc_main(int argc, char *argv[])
{
	tf->set_time_unit(1, SC_PS);

	mem.channels[0](rchannel);
	mem.channels[1](wchannel);

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
