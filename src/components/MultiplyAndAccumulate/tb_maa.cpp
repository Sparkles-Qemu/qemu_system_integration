// This file will be used as a test bench for the multiplu and accumulate
// component

// Necessary includes
#include "maa.cpp"
int sc_main(int argc, char* argv[]) {

    // ports 
    sc_signal <bool> enable;
    sc_signal <float > pixel;
    sc_signal <bool> reset;
    sc_signal <bool> clock;
    sc_signal <float > partialsum;
    sc_signal <float > out;
    float out_test;
    float weight = 3; //this can be an arbitrary number
    int error = 0;

    //instance and port binding
    mult_accumulate maa_ins("maa");

    maa_ins.pixelIn(pixel);
    maa_ins.enable(enable);
    maa_ins.reset(reset);
    maa_ins.clk(clock);
    maa_ins.partialSum(partialsum);
    maa_ins.output(out);
    maa_ins.weight = weight; //this is a local variable 


    sc_start(0, SC_NS);
    // open vcd file
    sc_trace_file *wf = sc_create_vcd_trace_file("mult_accumulate");
    sc_trace(wf, maa_ins.enable, "enable");
    sc_trace(wf, maa_ins.pixelIn, "pixel");
    sc_trace(wf, maa_ins.reset, "reset");
    sc_trace(wf, maa_ins.clk, "clk");
    sc_trace(wf, maa_ins.partialSum, "partials");
    sc_trace(wf, maa_ins.output, "output");
    sc_trace(wf, out_test, "out_test");
    

    // start test bench 
    enable = 1;

    for(int i = 0; i < 252; ++i){
        
        clock = 0;
        partialsum = i;
        pixel = 1;
        sc_start(1, SC_NS);
        out_test = pixel.read()*weight + partialsum.read();
        clock = 1;
        sc_start(1, SC_NS);
        if( out != out_test ) {
            error++;
            cout << "error ocurred inputs:"<<" pixel="<< pixel << " partialsum="<<partialsum << " out_test=" << out_test<< endl;
            cout <<"expected value =" << out << endl;
        }

    }

    //test reset
    reset = 0;
    clock = 0;
    sc_start(1, SC_NS);
    reset = 1;
    clock = 1;
    sc_start(1, SC_NS);
    if(out != 0) {
        error++;
        cout<< "reset did not work"<<" out ="<< out<< endl;
    }

    //error checking
    if( error != 0 ) {
        cout << "simulation falied"<< endl;
    } else {
        cout << "simulation passed" << endl;
    }

    sc_close_vcd_trace_file(wf);
    return 0;
}

