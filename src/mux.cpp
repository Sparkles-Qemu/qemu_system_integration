#include "systemc.h"
#include "vector"
#include <iostream>
SC_MODULE(mux)
{
    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_in<sc_int<8>> in;
    const int lines = 8;
    //sc_in<sc_int<8>> lines = 8;
    //sc_core::sc_vector<sc_core::sc_in<sc_int<8>> output_port;
    sc_vector<sc_out<sc_int<8>>> op;
    void init(){
        op.init(lines);
    }
    void MUX_(){
        init();
        int value = in.read();
        if(value == -1){
            for(int i =0;i<lines;i++){
                op[i].write(1);
            }
        }else{
        op[value].write(1);
        }
    }
    SC_CTOR(mux){
        SC_CTHREAD(MUX_,clk.pos());
        sensitive<<clk.pos();
        reset_signal_is(rst,true);
    }
    
}
