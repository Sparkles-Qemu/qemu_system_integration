#include "systemc.h"
#include "mux.cpp"
#include "tb.h"
#include <iostream>
SC_MODULE(SYSTEM){
    tb *ptb;
    mux *pmux;
    
    sc_signal<sc_int<8>> in_sig;
    sc_in<bool> rs_sig;
    sc_vector<sc_signal<sc_int<8>>> out_sig; 
    sc_clock clk_sig;

    SC_CTOR(SYSTEM):clk_sig("clk_sig",10,SC_NS)
    {
        ptb = new tb("ptb");
        ptb->clk(clk_sig);
        ptb->rst(rs_sig);
        ptb->input(in_sig);
        ptb->op(out_sig);
        pmux = new mux("pmux");
        pmux->clk(clk_sig);
        pmux->rst(rs_sig);
        pmux->in(in_sig);
        pmux->op(out_sig);

    }

}

int sc_main(int argc,char* argv[]){
    sc_start();
    //sc_trace_file *wf = sc_create_vcd_trace_file("./src/traces/sim_signals.trace");
    //sc_trace(wf, clk, "clk");
    //sc_trace(wf, rs_sig, "reset");
    //sc_trace(wf, in_sig, "clk");
    //sc_trace(wf, reset, "reset");
    return 0;
}