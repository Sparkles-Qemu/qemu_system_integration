#include <systemc.h>
#include <iostream>
#include <string>
SC_MODULE(mux) {
  sc_in<bool> clk{"clk"};
  sc_in<bool> reset{"reset"};
  sc_in<sc_uint<2> > input;
  sc_out<sc_uint<3> > output[3];
  int lines  = 3;
  void monitor(){
      cout << "@" << sc_time_stamp() <<"\no[0] : " << output[0] <<" o[1] : " << output[1] << " o[2] : " << output[2] << endl;

  }
  void body () {
    int i;
    int port_num = -1;
      if(reset.read()){
        i = 0;
        port_num = -1;
        for(i = 0;i<lines;i++)
        {
          output[i] = 0;
        }
        cout << "@" << sc_time_stamp() <<" reset applied  "<<endl;
      }else{
      if(port_num < lines){  
      port_num = input.read();
      cout << "@" << sc_time_stamp() <<" inputPort to write : " << port_num << endl;
      output[port_num].write(1);
      port_num = -1;
      }
      if(port_num > lines){
        cout << "@" << sc_time_stamp() <<" write to all ports : " << endl;
        for(i = 0;i<lines;i++){
          output[i] = port_num;
          port_num = -1;
        }

      }
      }
  }

  

  SC_CTOR(mux) {
    int j;
    SC_METHOD(body);
        sensitive << reset;
        sensitive << input;
    SC_METHOD(monitor);
        sensitive << output[0];
        sensitive << output[1];
        sensitive << output[2];
  }
};

// Testbench to generate test vectors
int sc_main (int argc, char* argv[]) {
  sc_signal< bool > clk;
  sc_signal< bool > reset;
  sc_signal<sc_uint<2> > inp;
  sc_signal<sc_uint<3> > o[3];

  int z;

  mux bind("mux");
  bind.clk(clk);
  bind.reset(reset);
  bind.input(inp);
  for (z=0; z<3; z++) {
      bind.output[z](o[z]);
    }
  sc_start(0,SC_NS);
  // Open VCD file
  sc_trace_file *wf = sc_create_vcd_trace_file("mux");
    sc_trace(wf,clk,"clk");
    sc_trace(wf,reset,"reset");
    sc_trace(wf,inp,"input");
    for (z=0; z<3; z++) {
      sc_trace(wf,o[z],"output");
    }
  // Start the testing here
  //sc_start(1, SC_NS);
  //reset = 0;
  //clk = 0;
  sc_start(1, SC_NS);
  reset = 1;
  clk = 1;
  reset = 0;
  sc_start(1, SC_NS);
  inp = rand()%3;
  //inp = 1;
  //sc_start(1);
  sc_start(1, SC_NS);
//  cout << "@" << sc_time_stamp() <<" input : " << inp << "\no[0] : " << o[0] <<" o[1] : " << o[1] << " o[2] : " << o[2] << endl;
  sc_start(2,SC_NS);
  //    //Test reset
    reset = 0;
    clk = 0;
    sc_start(1, SC_NS);
    reset = 1;
    clk = 1;
    sc_start(1, SC_NS);
//
  sc_close_vcd_trace_file(wf);
  return 0;// Terminate simulation
}
