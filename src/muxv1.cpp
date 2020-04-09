// All includes 
#include <systemc.h>



// Module defintion 
SC_MODULE (mux) {

    //inputs
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<bool> enable;
    sc_in<sc_uint<8> > in;

    //outputs
    //sc_vector<sc_out<sc_uint<8>>> out;
     sc_out<sc_uint<8> > out[8];


    //variable to hold # of o/p lines
     int lines = 8;

    void init(){
        //out.init(lines);
        cout<<"@"<< sc_time_stamp() << "Initializing\n" <<endl;
    }
    void MUX_(){
        init();
        int value = in.read();
        if(value == -1){
            for(int i =0;i<lines;i++){
                out[i].write(1);
            }
        }else{
        out[value].write(1);
        cout<<"@"<< sc_time_stamp() << "Output =" << out[value].read()<<endl;
        }
    }

    //function to monitor value changes on output value
    //void monitor() {
    //    cout<<"@"<< sc_time_stamp() << "Output =" << output.read()<<endl;
    //}


    //Constructor for module. This module is pos edge trigger 
    SC_CTOR(mux) {

        SC_METHOD(MUX_);
            sensitive << clk.pos();
        //SC_METHOD(monitor);
        //    sensitive << output;
    }

};





int sc_main(int argc, char* argv[]) {

    // Ports 

    sc_signal <bool> reset;
    sc_signal <bool> clock;
    sc_in<sc_uint<8> > input;
    //sc_vector<sc_signal<sc_uint<8>>> output;
    sc_signal<sc_uint<8>> output[8];
    //instance and port binding
    mux bind("mux");

    bind.reset(reset);
    bind.clk(clock);
    bind.in(input);
    int i;
    for(i=0;i<8;i++){
        bind.out[i](output[i]);
    }


    sc_start(0, SC_NS);
    // Open VCD file
    sc_trace_file *wf = sc_create_vcd_trace_file("mux");
    sc_trace(wf, bind.reset, "reset");
    sc_trace(wf, bind.clk, "clk");
    sc_trace(wf, bind.in, "input");
    for(int i =0;i<8;i++){
    sc_trace(wf, bind.out[i],"output");
    }
    

    // Start test bench 

    for(int i = 0; i < 8; ++i){
        
        clock = 0;
        sc_start(1, SC_NS);
        clock = 1;
        sc_start(1, SC_NS);
        output[i].write(i);
       

    }

    //Test reset
    reset = 0;
    clock = 0;
    sc_start(1, SC_NS);
    reset = 1;
    clock = 1;
    sc_start(1, SC_NS);

    sc_close_vcd_trace_file(wf);
    return 0;
}

