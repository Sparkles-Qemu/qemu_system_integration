#ifndef MEMORY_CPP // Note include guards, this is a quick and dirty way to include components
#define MEMORY_CPP

#include <systemc.h>
#include "map"
#include "vector"
#include <string>
#include <iostream>
#include <assert.h>

using std::cout;
using std::endl;
using std::fill;
using std::string;
using std::vector;

template <typename Type>
struct GenericCreator
{
    GenericCreator(sc_trace_file *_tf) : tf(_tf) {}

    Type *operator()(const char *name, size_t)
    {
        return new Type(name, tf);
    }
    sc_trace_file *tf;
};

struct GenericControlBus : public sc_module
{
    sc_signal<bool> clk, reset, enable;
    GenericControlBus(sc_module_name name, sc_trace_file *tf) : sc_module(name), clk("clk"), reset("reset"), enable("enable")    
    {
        sc_trace(tf, this->clk, (string(this->clk.name())));
        sc_trace(tf, this->reset, (string(this->reset.name())));
        sc_trace(tf, this->enable, (string(this->enable.name())));

        cout << "GENERIC CONTROL BUS MODULE: " << name << " has been instantiated " << endl;
    }
};

template <typename DataType>
struct MemoryBus : public sc_module
{
    sc_signal<DataType> data;
    sc_signal<unsigned int> addr;
    sc_signal<bool> enable;

    MemoryBus(sc_module_name name, sc_trace_file *tf) : sc_module(name), data("data"), addr("addr"), enable("enable")
    {
        sc_trace(tf, this->data, (string(this->data.name())));
        sc_trace(tf, this->addr, (string(this->addr.name())));
        sc_trace(tf, this->enable, (string(this->enable.name())));

        cout << "MEMORY BUS MODULE: " << name << " has been instantiated " << endl;
    }
};

template <typename DataType>
struct Port : public sc_module
{
    DataType data;
    sc_in<unsigned int> addr;
    sc_in<bool> enabled;

    Port(sc_module_name name, sc_trace_file *tf) : sc_module(name), data("data"), addr("addr"), enabled("enabled")
    {
        sc_trace(tf, this->data, (string(this->data.name())));
        sc_trace(tf, this->addr, (string(this->addr.name())));
        sc_trace(tf, this->enabled, (string(this->enabled.name())));

        cout << "MEMORY MODULE: " << name << " has been instantiated " << endl;
    }
};

template <typename DataType>
struct Memory : public sc_module
{
    // Control Signals
    sc_in_clk clk;
    sc_in<bool> reset, mem_enable;
    vector<DataType> ram;
    sc_vector<Port<sc_out<DataType>>> read_ports;
    sc_vector<Port<sc_in<DataType>>> write_ports;
    sc_vector<MemoryBus<DataType>> read_bus_bundle;
    sc_vector<MemoryBus<DataType>> write_bus_bundle;
    
    void update()
    {
        if (reset.read())
        {
            fill(ram.begin(), ram.end(), 0);
        }
        else if (mem_enable.read())
        {
            for (auto &&port : read_ports)
            {
                if (port.enabled.read())
                {
                    port.data.write(ram.at(port.addr.read()));
                }
            }

            for (auto &&port : write_ports)
            {
                if (port.enabled.read())
                {
                    ram.at(port.addr.read()) = port.data.read();
                }
            }
        }

    }



    // Constructor
    Memory(
        sc_module_name name,
        const GenericControlBus& _control,
        unsigned int _ram_size,
        unsigned int _read_port_count,
        unsigned int _write_port_count,
        sc_trace_file *tf) : sc_module(name), \
        ram(_ram_size, 0), \
        read_ports("read_port"), \
        write_ports("write_port"), \
        read_bus_bundle("read_bus"), \
        write_bus_bundle("write_bus")
    {
        this->clk(_control.clk);
        this->reset(_control.reset);
        this->mem_enable(_control.enable);

        read_ports.init(_read_port_count, GenericCreator<Port<sc_out<DataType>>>(tf));
        write_ports.init(_write_port_count, GenericCreator<Port<sc_in<DataType>>>(tf));
        read_bus_bundle.init(_read_port_count, GenericCreator<MemoryBus<DataType>>(tf));
        write_bus_bundle.init(_write_port_count, GenericCreator<MemoryBus<DataType>>(tf));

        for(unsigned int i = 0; i<_read_port_count; i++)
        {
            read_ports.at(i).data(read_bus_bundle.at(i).data);
            read_ports.at(i).addr(read_bus_bundle.at(i).addr);
            read_ports.at(i).enabled(read_bus_bundle.at(i).enable);
        }

        for(unsigned int i = 0; i<_write_port_count; i++)
        {
            write_ports.at(i).data(write_bus_bundle.at(i).data);
            write_ports.at(i).addr(write_bus_bundle.at(i).addr);
            write_ports.at(i).enabled(write_bus_bundle.at(i).enable);
        }

        SC_METHOD(update);
        sensitive << clk.pos();
        sensitive << reset;

        sc_trace(tf, this->clk, (string(this->name()) + string("_clk")));
        sc_trace(tf, this->reset, (string(this->name()) + string("_reset")));
        sc_trace(tf, this->mem_enable, (string(this->name()) + string("_mem_enable")));

        cout << "MEMORY MODULE: " << name << " has been instantiated " << endl;
    }

    SC_HAS_PROCESS(Memory);
};

#endif