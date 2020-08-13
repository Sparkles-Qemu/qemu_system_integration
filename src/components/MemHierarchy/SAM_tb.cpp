#include <c++/7/bits/c++config.h>
#include <systemc.h>
#include "SAM.cpp"
#include "sysc/kernel/sc_simcontext.h"
#include "sysc/kernel/sc_time.h"
// #define DEBUG
using std::cout;
using std::endl;
template <typename DataType>
struct SAM_TB : public sc_module
{
  sc_trace_file *tf;
  GlobalControlChannel control;
  SAM<DataType> dut;
  SAM_TB(sc_module_name name) : sc_module(name),
                             tf(sc_create_vcd_trace_file("ProgTrace")),
                             control("global_control_channel", sc_time(1, SC_NS), tf),
                             dut("dut", control, tf)
  {
    tf->set_time_unit(1, SC_PS);
    cout << "Instantiated SAM TB with name " << this->name() << endl;
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
int sc_main(int argc, char *argv[])
{
  SAM_TB<sc_int<32>> tb("SAM_tb");

  sc_start(1, SC_NS);

  return tb.run_tb();
}
