#ifndef GLOBAL_CONTROL_CPP // Note include guards, this is a quick and dirty way to include components
#define GLOBAL_CONTROL_CPP

#include <systemc.h>

struct GlobalControlChannel_IF : virtual public sc_interface
{
public:
    virtual sc_clock &clk() = 0;
    virtual const sc_signal<bool> &reset() = 0;
    virtual const sc_signal<bool> &program() = 0;
    virtual const sc_signal<bool> &enable() = 0;
    virtual void set_program(bool val) = 0;
    virtual void set_reset(bool val) = 0;
    virtual void set_enable(bool val) = 0;};

struct GlobalControlChannel : public sc_module, public GlobalControlChannel_IF
{
    sc_clock global_clock;
    sc_signal<bool> global_reset;
    sc_signal<bool> global_enable;
    sc_signal<bool> global_program;
    GlobalControlChannel(sc_module_name name,
                         sc_time time_val,
                         sc_trace_file *tf) : sc_module(name),
                                                       global_clock("clock", time_val),
                                                       global_reset("reset"),
                                                       global_enable("enable"),
                                                       global_program("program")
    {
        sc_trace(tf, this->global_clock, (this->global_clock.name()));
        sc_trace(tf, this->global_reset, (this->global_reset.name()));
        sc_trace(tf, this->global_program, (this->global_program.name()));
        sc_trace(tf, this->global_enable, (this->global_enable.name()));
    }

    sc_clock &clk()
    {
        return global_clock;
    }
    
    sc_signal<bool> &reset()
    {
        return global_reset;
    }

    sc_signal<bool> &program()
    {
        return global_program;
    }
    sc_signal<bool> &enable()
    {
        return global_enable;
    }

    void set_program(bool val)
    {
        global_program = val;
    }
    void set_reset(bool val)
    {
        global_reset = val;
    }

    void set_enable(bool val)
    {
        global_enable = val;
    }
};


#endif