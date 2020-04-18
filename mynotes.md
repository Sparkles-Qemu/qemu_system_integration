SYSTEM C notes:
BASIC MODEL OF A SYSTEM C PROGRAM:

SC_MODULE(prototype_name){

sc_in<DT> input_port1; //read from input port using input_port.read();
sc_in<DT> input_port2;
sc_out<DT> output_port; //write to an output_port using output_port.write();
sc_in<bool> clk;

void and_op(){
    output_port.write(input_port1.read() & input_port2.read());
}
SC_CTOR(PROTOTYPE_NAME){
    //default constructor
    SC_METHOD(and_op);
    sensitive << clk.pos();
}
}

//THREADS IN SYSTEM C
By defualt threads are concurrently run and always active

3 kinds of threads:
1.SC_METHOD()
    -Runs continously and gets called once every time a sensitive event occurs
    -Can be synthesized

2.SC_THREAD()
    - Runs once at the start of simulation
    - Not synthesizable

3.SC_CTHREAD()

    -Runs continously and references a clock edge and also synthesizable
    -Most commonly used