#ifndef MEMORY_CPP // Note include guards, this is a quick and dirty way to include components
#define MEMORY_CPP

#include <systemc.h>
#include "map"
#include "vector"
#include <string>
#include <iostream>
#include <assert.h>
#include <Connector.cpp>
#include <algorithm> // for copy() and assign()
#include <iterator>  // for back_inserter

using std::back_inserter;
using std::copy;
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

enum MemoryChannelMode
{
    READ,
    WRITE
};

struct GlobalControlChannel_IF : virtual public sc_interface
{
public:
    virtual sc_clock &clk() = 0;
    virtual const sc_signal<bool> &reset() = 0;
    virtual const sc_signal<bool> &enable() = 0;
    virtual void set_reset(bool val) = 0;
    virtual void set_enable(bool val) = 0;};

struct GlobalControlChannel : public sc_module, public GlobalControlChannel_IF
{
    sc_clock global_clock;
    sc_signal<bool> global_reset;
    sc_signal<bool> global_enable;
    GlobalControlChannel(sc_module_name name,
                         sc_time time_val,
                         sc_trace_file *tf) : sc_module(name),
                                                       global_clock("clock", time_val),
                                                       global_reset("reset"),
                                                       global_enable("enable")
    {
        sc_trace(tf, this->global_clock, (this->global_clock.name()));
        sc_trace(tf, this->global_reset, (this->global_reset.name()));
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

    sc_signal<bool> &enable()
    {
        return global_enable;
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

template <typename DataType>
struct MemoryChannel_IF : virtual public sc_interface
{
public:
    virtual const MemoryChannelMode& mode() = 0;
    virtual void set_mode(MemoryChannelMode mode) = 0;
    virtual const sc_vector<sc_signal<DataType>>& read_data() = 0;
    virtual void write_data(const sc_vector<sc_signal<DataType>> & _data) = 0;
    virtual void write_data_element(DataType _data, unsigned int col) = 0;
    virtual unsigned int addr() = 0;
    virtual void set_addr(unsigned int addr) = 0;
    virtual bool enabled() = 0;
    virtual void set_enable(bool status) = 0;
    virtual void reset() = 0;
};

template <typename DataType>
struct MemoryChannel : public sc_module, public MemoryChannel_IF<DataType>
{
    sc_vector<sc_signal<DataType>> channel_data;
    sc_signal<unsigned int> channel_addr;
    sc_signal<bool> channel_enabled;
    sc_signal<MemoryChannelMode> channel_mode;
    unsigned int channel_width;

    MemoryChannel(sc_module_name name, unsigned int width, sc_trace_file *tf) : sc_module(name), \
    channel_data("data", width), \
    channel_addr("addr"), \
    channel_enabled("enabled"), \
    channel_mode("mode")
    {
        channel_addr = 0;
        channel_enabled = false;
        channel_mode = MemoryChannelMode::READ;
        channel_width = width;
        // // sc_trace(tf, this->data, (string(this->data.name())));
    }

    const sc_vector<sc_signal<DataType>>& read_data()
    {
        return channel_data;
    }

    void write_data(const sc_vector<sc_signal<DataType>> &_data)
    {
        assert(_data.size() == channel_data.size());
        for(unsigned int i = 0; i<channel_width; i++)
        {
            channel_data[i] = _data[i];
        }
    }

    void write_data_element(DataType _data, unsigned int col)
    {
        assert(col <= channel_data.size() && col >= 0);
        channel_data[col] = _data;
    }

    unsigned int addr()
    {
        return channel_addr;
    }

    void set_addr(unsigned int addr)
    {
        channel_addr = addr;
    }

    void set_enable(bool status)
    {
        channel_enabled = status;
    }
    bool enabled()
    {
        return channel_enabled;
    }

    void set_mode(MemoryChannelMode mode)
    {
        channel_mode = mode;
    }

    const MemoryChannelMode& mode()
    {
        return channel_mode.read();
    }

    void reset()
    {
        for (auto &data : channel_data)
        {
            data = 0;
        }
        channel_addr = 0;
        channel_enabled = false;
        channel_mode = MemoryChannelMode::READ;
    }

    void register_port(sc_port_base &port_,
                       const char *if_typename_)
    {
        cout << "binding    " << port_.name() << " to "
             << "interface: " << if_typename_ << " with channel width " << channel_width << endl;
    }
};

template <typename DataType>
struct MemoryChannelCreator
{
    MemoryChannelCreator(unsigned int _width, sc_trace_file *_tf) : tf(_tf), width(_width) {}

    MemoryChannel<DataType> *operator()(const char *name, size_t)
    {
        return new MemoryChannel<DataType>(name, width, tf);
    }
    sc_trace_file *tf;
    unsigned int width;
};

template <typename DataType>
struct MemoryRowCreator
{
    MemoryRowCreator(unsigned int _width, sc_trace_file *_tf) : tf(_tf), width(_width) {}

    sc_vector<sc_signal<DataType>> *operator()(const char *name, size_t)
    {
        return new sc_vector<sc_signal<DataType>>(name, width);
    }
    sc_trace_file *tf;
    unsigned int width;
};

template <typename DataType>
struct Memory : public sc_module
{
    // Control Signals
    unsigned int length, width;
    sc_vector<sc_vector<sc_signal<DataType>>> ram;
    sc_port<GlobalControlChannel_IF> control;
    sc_vector<sc_port<MemoryChannel_IF<DataType>>> channels;

    void update()
    {
        if (control->reset())
        {
            for (auto &row : ram)
            {
                for (auto &col : row)
                {
                    col = 0;
                }
            }
        }
        else if (control->enable())
        {
            for (auto &channel : channels)
            {
                if (channel->enabled())
                {
                    switch (channel->mode())
                    {
                    case MemoryChannelMode::WRITE:
                        assert(channel->read_data().size() == width);
                        for(unsigned int i = 0; i<width; i++)
                        {
                            ram[channel->addr()][i] = channel->read_data()[i];
                        }
                        break;

                    case MemoryChannelMode::READ:
                        assert(ram[channel->addr()].size() == width);
                        channel->write_data(ram[channel->addr()]);
                        break;
                    }
                }
            }
        }
    }

    void print_memory_contents()
    {
        for (const auto& row : ram)
        {
            for(const auto& col : row)
            {
                cout << col << " ";
            }
            cout << endl;
        }

    }

    // Constructor
    Memory(
        sc_module_name name,
        GlobalControlChannel& _control,
        unsigned int _channel_count,
        unsigned int _length,
        unsigned int _width,
        sc_trace_file *tf) : sc_module(name),
                             ram("ram", _length, MemoryRowCreator<DataType>(_width,tf)),
                             control("control"),
                             channels("channel", _channel_count)
    {
        length = _length;
        width = _width;

        control(_control);

        SC_METHOD(update);
        sensitive << control->clk();
        sensitive << control->reset();

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