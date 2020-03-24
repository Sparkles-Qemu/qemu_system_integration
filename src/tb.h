#include <systemc.h>

SC_MODULE(tb)
{
    sc_in<bool> clk;
    sc_out<bool> rst;
    sc_out<sc_int<8>> input;
    sc_vector<sc_in<sc_int<8>>> op;

    void source();
    void sink();
    SC_CTOR(tb){
        SC_CTHREAD(source,clk.pos());
        SC_CTHREAD(sink,clk.pos());
    }
}