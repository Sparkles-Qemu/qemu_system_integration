#include "systemc.h"
#include "DMA.cpp"

using std::cout;
using std::endl;

#define MAX_RESET_CYCLES 10
#define MAX_SIM_CYCLES 10

/**
 * @brief Simulation entry point note sc_main not main.... 
 * a systemc idiosyncrasy related to the linking behavior. 
 * 
 * @param argc number of arguments passed (just like regular C )
 * @param argv arguments passed (just like regular C )
 * @return int 
 */
int sc_main(int argc, char *argv[]) 
{
    sc_signal<bool > clk("clk"); 
    sc_signal<bool > reset("reset"); 
    sc_signal<bool > enable("enable"); 

    // base DMA instantiation
    DMA_Base dma_base("dma_base", &clk, &reset, &enable);

    /**
     * @brief Here's how you log specific signals you would want to watch in
     * gtkwave. Just call sc_trace on whatever signal you want and give it a
     * name so that you can find it in the signal heirarchy in gtkwave.
     * 
     */

    sc_trace_file *wf = sc_create_vcd_trace_file("./traces/sim_signals.trace");
    sc_trace(wf, clk, "clk");
    sc_trace(wf, reset, "reset");

    reset = 0;
    enable = 1;
    /**
     * @brief Steps the simulation one ns forward. The SC_NS value is an enum
     * and it can be milliseconds or seconds. 
     * 
     */
    sc_start(1, SC_NS);
    reset = 1;
    cout << "@ " << sc_time_stamp() << " Asserting reset" << endl;
    for (int i = 0; i < MAX_RESET_CYCLES; i++)
    {
        /**
         * @brief Since rest is level triggered, update will only be called once
         * 
         */
        sc_start(1, SC_NS); 
    }
    cout << "@ " << sc_time_stamp() << " Deasserting reset" << endl;
    reset = 0; 
    
    cout << "@ " << sc_time_stamp() << " Start Compute" << endl;
    for (int i = 0; i < MAX_SIM_CYCLES; i++)
    {
        clk = 1;
        sc_start(1, SC_NS);
        clk = 0;
        sc_start(1, SC_NS);
    }

    cout << "@ " << sc_time_stamp() << " Done with compute, testing async reset" << endl;
    
    /**
     * @brief Random reset not tied to a particular clk cycle. Note change in precision
     * when outputing sim time in module.
     * 
     */
    cout << "@ " << sc_time_stamp() << " Compute complete, testing async reset" << endl;
    sc_start(1.5, SC_NS); 
    reset = 1;
    sc_start(1.5, SC_NS);
    
    cout << "@ " << sc_time_stamp() << " Sim complete, Simulation terminating .... " << endl;


    return 0; // Terminate simulation
}

