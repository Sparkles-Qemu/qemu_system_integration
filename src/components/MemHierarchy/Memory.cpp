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

enum class DescriptorState
{
  SUSPENDED, // do nothing indefinitely
  GENERATE,  // transfer data
  WAIT       // do nothing for certain number of cycles
};

struct Descriptor_2D
{
    unsigned int next;     // index of next descriptor
    unsigned int start;    // start index in ram array
    DescriptorState state;        // state of dma
    unsigned int x_count;  // number of floats to transfer/wait
    unsigned int x_modify; // number of floats between each transfer/wait
    unsigned int y_count;  // number of floats to transfer/wait
    unsigned int y_modify; // number of floats between each transfer/wait
};

template <typename DataType>
struct AddressGenerator : public sc_module
{
    // Control Signals
    sc_in<bool> clk, reset, enable;

    sc_in<DataType> data;
    sc_out<unsigned int> addr;
    sc_out<bool> port_enable;

    // Internal Data
    vector<Descriptor_2D> descriptors;
    unsigned int execute_index;
    unsigned int current_ram_index;
    unsigned int x_count_remaining;
    unsigned int y_count_remaining;

    const Descriptor_2D default_descriptor = {0, 0, DescriptorState::SUSPENDED, 0, 0, 0, 0};

    void resetIndexingCounters()
    {
        x_count_remaining = descriptors[execute_index].x_count;
        y_count_remaining = descriptors[execute_index].y_count;
    }

    void resetAllInternalCounters()
    {
        current_ram_index = descriptors[execute_index].start;
        x_count_remaining = descriptors[execute_index].x_count;
        y_count_remaining = descriptors[execute_index].y_count;
    }

    void loadProgram(vector<Descriptor_2D> newProgram)
    {
        descriptors.clear();
        copy(newProgram.begin(), newProgram.end(), std::back_inserter(descriptors));
        execute_index = 0;
        resetAllInternalCounters();
    }

    void resetProgramMemory()
    {
        descriptors.clear();
        descriptors.push_back(default_descriptor);
        execute_index = 0;
    }

    Descriptor_2D currentDescriptor()
    {
        return descriptors[execute_index];
    }

    void updateCurrentIndex()
    {
        x_count_remaining--;
        if (x_count_remaining == 0)
        {
            if (y_count_remaining != 0)
            {
                current_ram_index += currentDescriptor().y_modify;
                x_count_remaining = currentDescriptor().x_count;
                y_count_remaining--;
            }
            else
            {
                /* DO NOTHING, NEED TO EXECUTE NEXT DESCRIPTOR */
            }
        }
        else
        {
            current_ram_index += currentDescriptor().x_modify;
        }
    }

    bool descriptorComplete()
    {
        return (x_count_remaining == 0 && y_count_remaining == 0);
    }

    void loadNextDescriptor()
    {
        execute_index = descriptors[execute_index].next;
        resetAllInternalCounters();
    }

    // Called on rising edge of clk or high level reset
    void update()
    {
        if (reset.read())
        {
            resetProgramMemory();
            resetAllInternalCounters();
            std::cout << "@ " << sc_time_stamp() << " " << this->name() << ":MODULE has been reset" << std::endl;
        }
        else if (enable.read())
        {
            switch (currentDescriptor().state)
            {
            case DescriptorState::GENERATE:
            {
                port_enable.write(true);
                addr.write(current_ram_index);
                updateCurrentIndex();
                if (descriptorComplete())
                {
                    loadNextDescriptor();
                }
                break;
            }
            case DescriptorState::WAIT:
            {
                port_enable.write(false);
                updateCurrentIndex();
                if (descriptorComplete())
                {
                    loadNextDescriptor();
                }
                break;
            }
            case DescriptorState::SUSPENDED:
            {
                port_enable.write(false);
                break;
            }
            default:
            {
                std::cout << "@ " << sc_time_stamp() << " " << this->name() << ": Is in an invalid state! ... exitting" << std::endl;
                exit(-1);
            }
            }
        }
    }

    // Constructor
    AddressGenerator(sc_module_name name, const GenericControlBus& _control) : sc_module(name)
    {
        SC_METHOD(update);
        sensitive << reset;
        sensitive << clk.pos();

        // connect signals
        this->clk(_control.clk);
        this->reset(_control.reset);
        this->enable(_control.enable);
        std::cout << "ADDRESS_GENERATOR MODULE: " << name << " has been instantiated " << std::endl;
    }

    SC_HAS_PROCESS(AddressGenerator);
};

//TODO: create generic connector that takes 2 sc_in_interface references and binds them
// with a sc_signal of whatever type (depending on the ports), the sc_signal should be named
// and added to the vector 

template <typename DataType>
struct SAM : public sc_module
{
    // Control Signals
    Memory<DataType> memory;
    sc_vector<sc_out<DataType>> data_read_ports;
    sc_vector<sc_in<DataType>> data_write_ports;
    sc_vector<AddressGenerator<DataType>> address_generators;

    SAM(
        sc_module_name name,
        const GenericControlBus& _control,
        unsigned int _ram_size,
        unsigned int _read_port_count,
        unsigned int _write_port_count,
        /*TODO: add initalizer list for data bus mapping for each generator attached
        to input port, must include default that connects to default 0th bus*/
        sc_trace_file *tf) : sc_module(name), \
        memory("memory", _control, _ram_size, _read_port_count, _write_port_count, tf)
    {

        data_read_ports.init(_read_port_count, "data_read_port");
        data_write_ports.init(_write_port_count, "data_write_port");

        /*TODO: use generic connector (instantiated as member here) to hook up
        address generators*/

        sc_trace(tf, this->clk, (string(this->name()) + string("_clk")));
        sc_trace(tf, this->reset, (string(this->name()) + string("_reset")));
        sc_trace(tf, this->mem_enable, (string(this->name()) + string("_mem_enable")));

        cout << "SAM MODULE: " << name << " has been instantiated " << endl;
    }

    SC_HAS_PROCESS(SAM);
};

#endif