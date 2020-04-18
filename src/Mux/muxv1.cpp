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
