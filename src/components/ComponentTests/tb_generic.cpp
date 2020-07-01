#include <systemc.h>
#include <iostream>

using std::cout;
using std::endl;

struct WaitTester : public sc_module 
{
    //inputs
    sc_in<bool> clock ;
    unsigned int counter = 0;

    void update() 
    {
        while(true)
        {
            if(counter % 2 == 0)
            {
                cout<<"@"<< sc_time_stamp() << " Counter Increment! Current Counter: " << counter << endl;
                counter++;
            }
            else
            {
                cout<<"@"<< sc_time_stamp() << " HAVE TO WAIT! Counter: " << counter << endl;
                counter++;
                wait();
            }
            cout<<"@"<< sc_time_stamp() << " WAITING FOR NEXT CYCLE : " << endl;            
            wait();
        }

    }

    WaitTester(
            sc_module_name name, 
            const sc_signal<bool> &_clock
        ) : sc_module(name)
    {
        SC_THREAD(update);
        sensitive << clock.pos();

        this->clock(_clock);
    }

    SC_HAS_PROCESS(WaitTester);

};

int sc_main(int argc, char *argv[])
{
    sc_signal<bool> clk("clk");
    WaitTester waiter("WAITER", clk);

	for (unsigned int i = 0; i < 20; i++)
	{
		clk = 1;
		sc_start(0.5, SC_NS);
		clk = 0;
		sc_start(0.5, SC_NS);
	}
    return 0;
}