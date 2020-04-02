// All includes 
#include <systemc.h>

// My defines 



// Module defintion 
SC_MODULE (mult_accumulate) {

    //inputs
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<bool> enable;
    sc_in<sc_uint<8> > partialSum;
    sc_in<sc_uint<8> > pixelIn;

    //outputs
    sc_out<sc_uint<8> > output;


    //local variable to store weight
    sc_uint<8> weight;
    sc_uint<8> result;

    //function to update the output value
    void update() {

        if(reset.read() == 1) {
            output.write(0);
        } else if (enable.read() == 1) {
            result = pixelIn.read() * weight + partialSum.read();
            output.write(result);
        }
    }


    //function to monitor value changes on output value
    void monitor() {
        cout<<"@"<< sc_time_stamp() << "Output =" << output.read()<<endl;
    }


    //Constructor for module. This module is pos edge trigger 
    SC_CTOR(mult_accumulate) {

        SC_METHOD(update);
            dont_initialize();
            sensitive << clk.pos();
        SC_METHOD(monitor);
            sensitive << output;
    }

};
