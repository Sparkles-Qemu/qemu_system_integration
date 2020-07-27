#ifndef MEMORY_CPP // Note include guards, this is a quick and dirty way to include components
#define MEMORY_CPP

#include <systemc.h>
#include "map"
#include "vector"
#include <string>
#include <iostream>
#include <assert.h>
#include <Connector.cpp>

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

enum class MemoryChannelMode
{
    READ,
    WRITE,
    HIGH_IMPEDANCE
};

template <typename DataType>
struct MemoryChannel_IF : virtual public sc_interface
{
public:
    virtual MemoryChannelMode mode() = 0;
    virtual const sc_vector<DataType>& read_data() = 0;
    virtual void write_data(const sc_vector<DataType>& _data) = 0;
    virtual unsigned int addr() = 0; // write a character
    virtual bool enabled() = 0;
    virtual void reset() = 0; // empty the stack
};

template <typename DataType>
struct MemoryChannel : public sc_module, public MemoryChannel_IF<DataType>
{
    sc_vector<DataType> data;
    unsigned int addr;
    bool enabled;
    MemoryChannelMode mode;

    MemoryChannel(sc_module_name name, unsigned int width, sc_trace_file* tf) : sc_module(name)
    {
        data.init("data", width);
        addr = 0;
        enabled = false;
        mode = MemoryChannelMode::HIGH_IMPEDANCE;
    
    }

    const sc_vector<DataType>& read_data()
    {
        return data;
    }
    
    void write_data(const sc_vector<DataType>& _data)
    {
        assert(_data.size() == data.size());
        for(int i = 0; i<data.size(); i++)
        {
            data[i] = _data[i];
        }
    }

    unsigned int addr()
    {
        return addr;
    }

    bool enabled()
    {
        return enabled;
    }

    MemoryChannelMode mode()
    {
        return mode;
    }
};

template <typename DataType>
struct Memory : public sc_module
{
    // Control Signals
    sc_in_clk clk;
    sc_in<bool> reset, mem_enable;
    sc_vector<sc_vector<DataType>> ram;
    sc_vector<sc_port<MemoryChannel_IF<DataType>>> channels;

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
        const GenericControlBus &_control,
        unsigned int _read_port_count,
        unsigned int _write_port_count,
        sc_trace_file *tf) : sc_module(name),
                             ram("ram", length),
                             read_ports("read_port"),
                             write_ports("write_port"),
                             read_bus_bundle("read_bus"),
                             write_bus_bundle("write_bus")
    {
        this->clk(_control.clk);
        this->reset(_control.reset);
        this->mem_enable(_control.enable);

        read_ports.init(_read_port_count, GenericCreator<Port<sc_out<DataType>>>(tf));
        write_ports.init(_write_port_count, GenericCreator<Port<sc_in<DataType>>>(tf));
        read_bus_bundle.init(_read_port_count, GenericCreator<MemoryBus<DataType>>(tf));
        write_bus_bundle.init(_write_port_count, GenericCreator<MemoryBus<DataType>>(tf));

        for (unsigned int i = 0; i < _read_port_count; i++)
        {
            read_ports.at(i).data(read_bus_bundle.at(i).data);
            read_ports.at(i).addr(read_bus_bundle.at(i).addr);
            read_ports.at(i).enabled(read_bus_bundle.at(i).enable);
        }

        for (unsigned int i = 0; i < _write_port_count; i++)
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
    DescriptorState state; // state of dma
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
    AddressGenerator(sc_module_name name, const GenericControlBus &_control) : sc_module(name)
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

#endif