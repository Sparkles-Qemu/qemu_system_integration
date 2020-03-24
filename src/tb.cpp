#include "tb.h"
#include <iostream>
void tb::source(){
    input.write(0);
    rst.write(1);
    wait();
    rst.write(0);
    wait();
    //sc_int<8> value;
    for(int i =0;i<8;i++){
        input.write(i);
    }
}

void tb::sink(){
     sc_int<8> value;
    for(int i =0;i<8;i++){
     value = op[i].read();
     wait();
     cout<<i<<value<<endl;
    }

    sc_stop();
    
}