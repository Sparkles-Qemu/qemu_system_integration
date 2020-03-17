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
            sensitive << clk.pos();
        SC_METHOD(monitor);
            sensitive << output;
    }

};





int sc_main(int argc, char* argv[]) {

    // Ports 
    sc_signal <bool> enable;
    sc_signal <sc_uint<8> > pixel;
    sc_signal <bool> reset;
    sc_signal <bool> clock;
    sc_signal <sc_uint<8> > partialSum;
    sc_signal <sc_uint<8> > out;
    sc_uint<8> out_test;
    sc_uint<8> weight = 3; //this can be an arbitrary number
    int error = 0;

    //instance and port binding
    mult_accumulate maa_ins("MAA");

    maa_ins.pixelIn(pixel);
    maa_ins.enable(enable);
    maa_ins.reset(reset);
    maa_ins.clk(clock);
    maa_ins.partialSum(partialSum);
    maa_ins.output(out);
    maa_ins.weight = weight; //this is a local variable 


    sc_start(0, SC_NS);
    // Open VCD file
    sc_trace_file *wf = sc_create_vcd_trace_file("mult_accumulate");
    sc_trace(wf, maa_ins.enable, "enable");
    sc_trace(wf, maa_ins.pixelIn, "pixel");
    sc_trace(wf, maa_ins.reset, "reset");
    sc_trace(wf, maa_ins.clk, "clk");
    sc_trace(wf, maa_ins.partialSum, "partialS");
    sc_trace(wf, maa_ins.output, "output");
    

    // Start test bench 
    enable = 1;

    for(int i = 0; i < 252; ++i){
        
        clock = 0;
        sc_start(1, SC_NS);
        clock = 1;
        sc_start(1, SC_NS);
        partialSum = i;
        pixel = 1;
        out_test = pixel.read()*weight + partialSum.read();
        if( out != out_test ) {
            error++;
            cout << "Error ocurred inputs:"<<" pixel="<< pixel << " partialSum="<<partialSum << " out_test=" << out_test<< endl;
            cout <<"Expected value =" << out << endl;
        }

    }

    //Test reset
    reset = 0;
    clock = 0;
    sc_start(1, SC_NS);
    reset = 1;
    clock = 1;
    sc_start(1, SC_NS);
    if(out != 0) {
        error++;
        cout<< "Reset did not work"<<" Out ="<< out<< endl;
    }

    //error checking
    if( error != 0 ) {
        cout << "Simulation falied"<< endl;
    } else {
        cout << "Simulation passed" << endl;
    }

    sc_close_vcd_trace_file(wf);
    return 0;
}

