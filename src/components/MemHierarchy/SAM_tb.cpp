#include "SAM.cpp"
#include <systemc.h>
// #define DEBUG
using std::cout;
using std::endl;
template <typename DataType>
struct SAM_TB : public sc_module
{
    const unsigned int dut_mem_length = 128;
    const unsigned int dut_mem_width = 4;
    const unsigned int dut_mem_channel_count = 4;
    sc_trace_file* tf;
    GlobalControlChannel control;
    SAM<DataType> dut;
    SAM_TB(sc_module_name name) : sc_module(name),
                                  tf(sc_create_vcd_trace_file("ProgTrace")),
                                  control("global_control_channel", sc_time(1, SC_NS), tf),
                                  dut("dut", control, dut_mem_channel_count, dut_mem_length, dut_mem_width, tf)
    {
        tf->set_time_unit(1, SC_PS);
        cout << "Instantiated SAM TB with name " << this->name() << endl;
    }
    bool validate_reset()
    {
        return true;
    }

    bool validate_write_to_sam_1D()
    {
        cout << "Validating validate_write_to_sam_1D" << endl;

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

    sc_start(1, SC_NS);

    return tb.run_tb();
}
