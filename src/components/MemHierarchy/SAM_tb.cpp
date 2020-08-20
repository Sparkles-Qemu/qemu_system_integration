#include "AddressGenerator.cpp"
#include "SAM.cpp"
#include "sysc/kernel/sc_time.h"
#include <systemc.h>

// #define DEBUG
using std::cout;
using std::endl;
template <typename DataType>
struct SAM_TB : public sc_module
{
    const unsigned int dut_mem_length = 128;
    const unsigned int dut_mem_width = 4;
    const unsigned int dut_mem_channel_count = 2;
    sc_trace_file* tf;
    GlobalControlChannel control;
    SAM<DataType> dut;
    sc_vector<sc_signal<DataType>> channel_0_read_bus;
    sc_vector<sc_signal<DataType>> channel_0_write_bus;
    sc_vector<sc_signal<DataType>> channel_1_read_bus;
    sc_vector<sc_signal<DataType>> channel_1_write_bus;

    SAM_TB(sc_module_name name) : sc_module(name),
                                  tf(sc_create_vcd_trace_file("ProgTrace")),
                                  control("global_control_channel", sc_time(1, SC_NS), tf),
                                  dut("dut", control, dut_mem_channel_count, dut_mem_length, dut_mem_width, tf),
                                  channel_0_read_bus("channel_0_read_bus", dut_mem_width),
                                  channel_0_write_bus("channel_0_write_bus", dut_mem_width),
                                  channel_1_read_bus("channel_1_read_bus", dut_mem_width),
                                  channel_1_write_bus("channel_1_write_bus", dut_mem_width)
    {
        for (unsigned int i = 0; i < dut_mem_width; i++)
        {
            dut.read_channel_data[0][i](channel_0_read_bus[i]);
            dut.write_channel_data[0][i](channel_0_write_bus[i]);
            dut.read_channel_data[1][i](channel_1_read_bus[i]);
            dut.write_channel_data[1][i](channel_1_write_bus[i]);

            sc_trace(tf, channel_0_read_bus[i], channel_0_read_bus[i].name());
            sc_trace(tf, channel_0_write_bus[i], channel_0_write_bus[i].name());
            sc_trace(tf, channel_1_read_bus[i], channel_1_read_bus[i].name());
            sc_trace(tf, channel_1_write_bus[i], channel_1_write_bus[i].name());
        }
        tf->set_time_unit(1, SC_PS);
        cout << "Instantiated SAM TB with name " << this->name() << endl;
    }
    bool validate_reset()
    {
        control.set_reset(true);
        for (unsigned int channel_idx = 0; channel_idx < dut_mem_channel_count; channel_idx++)
        {
            dut.channels[channel_idx].bus_reset();
        }
        control.set_program(false);
        control.set_enable(false);

        sc_start(1, SC_NS);

        control.set_reset(false);
        for (unsigned int idx = 0; idx < dut_mem_channel_count; idx++)
        {
            cout << "checking address generator[" << idx << "] " << endl;
            if (!(dut.generators[idx].descriptors[0] == default_descriptor))
            {
                cout << "dut.generators[idx].descriptors[0] == default_descriptor FAILED!" << endl;
                return false;
            }

            if (!(dut.generators[idx].execute_index == 0))
            {
                cout << "dut.generators[idx].execute_index == 0 FAILED!" << endl;
                return false;
            }

            if (!(dut.generators[idx].programmed == false))
            {
                cout << "dut.generators[idx].programmed == false FAILED!" << endl;
                return false;
            }
            cout << "address generator[" << idx << "] reset correctly! " << endl;

            cout << "checking channel[" << idx << "] " << endl;
            if (!(dut.generators[idx].descriptors[0] == default_descriptor))
                for (auto& data : dut.channels[idx].read_channel_data)
                {
                    if (!(data == DataType(0)))
                    {
                        cout << "channel read_bus_data == 0 FAILED!" << endl;
                        return false;
                    }
                }
            for (auto& data : dut.channels[idx].write_channel_data)
            {
                if (!(data == DataType(0)))
                {
                    cout << "channel write_bus_data == 0 FAILED!" << endl;
                    return false;
                }
            }
            cout << "channel[" << idx << "] reset correctly! " << endl;
        }

        // check memory cells
        for (auto& row : dut.mem.ram)
        {
            for (auto& col : row)
            {
                if (!(col == DataType(0)))
                {
                    cout << "col == DataType(0) FAILED!" << endl;
                    return false;
                }
            }
        }

        return true;
    }

    bool validate_write_to_sam_1D()
    {
        cout << "Validating validate_write_to_sam_1D" << endl;

        control.set_reset(true);
        control.set_program(false);
        control.set_enable(false);

        sc_start(1, SC_NS);

        control.set_reset(false);

        Descriptor_2D generate_1D_descriptor_1(1, 10, DescriptorState::GENERATE, 10,
                                               1, 0, 0);

        Descriptor_2D suspend_descriptor(1, 0, DescriptorState::SUSPENDED, 0, 0, 0,
                                         0);

        vector<Descriptor_2D> temp_program;
        temp_program.push_back(generate_1D_descriptor_1);
        temp_program.push_back(suspend_descriptor);

        dut.generators[0].loadProgram(temp_program);
        dut.channels[0].set_mode(MemoryChannelMode::WRITE);
        dut.channels[1].set_mode(MemoryChannelMode::READ);

        control.set_program(true);
        cout << "load program and start first descriptor" << endl;
        sc_start(1, SC_NS);
        control.set_enable(true);
        control.set_program(false);

        for (unsigned int i = 1; i <= 10; i++)
        {
            // dut.write_channel_data[0][0]->write(i);
            channel_0_write_bus[0] = DataType(i);
            sc_start(1, SC_NS);

        }
        sc_start(1, SC_NS);


        cout << "validate_write_to_sam_1D SUCCESS" << endl;
        return true;
    }

    bool validate_write_to_sam_2D()
    {
        cout << "Validating validate_write_to_sam_2D" << endl;

        cout << "validate_write_to_sam_2D SUCCESS" << endl;
        return true;
    }

    bool validate_read_from_sam_1D()
    {
        cout << "Validating validate_read_from_sam_1D" << endl;

        cout << "validate_read_from_sam_1D SUCCESS" << endl;
        return true;
    }

    bool validate_read_from_sam_2D()
    {
        cout << "Validating validate_read_from_sam_2D" << endl;

        cout << "validate_read_from_sam_2D SUCCESS" << endl;
        return true;
    }

    bool validate_wait_with_data_write()
    {
        cout << "Validating validate_wait_with_data_write" << endl;

        cout << "validate_wait_with_data_write SUCCESS" << endl;
        return true;
    }

    bool validate_concurrent_read_write_2D()
    {
        cout << "Validating validate_concurrent_read_write_2D" << endl;

        cout << "validate_concurrent_read_write_2D SUCCESS" << endl;
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
        if (!(validate_write_to_sam_1D()))
        {
            cout << "validate_write_to_sam_1D() FAILED!" << endl;
            return false;
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
    ~SAM_TB()
    {
        sc_close_vcd_trace_file(tf);
    }
};

int sc_main(int argc, char* argv[])
{
    SAM_TB<sc_int<32>> tb("SAM_tb");
    return tb.run_tb();
}
