#ifndef MEMORY_CPP // Note include guards, this is a quick and dirty way to include components
#define MEMORY_CPP

#include <systemc.h>
#include <string>
#include <iostream>
#include <assert.h>
#include <GlobalControl.cpp>

using std::cout;
using std::endl;
using std::string;

enum MemoryChannelMode
{
    READ,
    WRITE
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

        for(unsigned int i = 0; i<channel_width; i++)
        {
            sc_trace(tf, this->channel_data[i], (string(this->channel_data[i].name())));
        }

        sc_trace(tf, this->channel_addr, (string(this->channel_addr.name())));
        sc_trace(tf, this->channel_enabled, (string(this->channel_enabled.name())));
        sc_trace(tf, this->channel_mode, (string(this->channel_mode.name())));
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

        for(unsigned int row = 0; row < length; row++)
        {
            for(unsigned int col = 0; col < width; col++)
            {
                sc_trace(tf, ram[row][col], ram[row][col].name());
            }
        }

        control(_control);

        SC_METHOD(update);
        sensitive << control->clk();
        sensitive << control->reset();

        cout << "MEMORY MODULE: " << name << " has been instantiated " << endl;
    }

    SC_HAS_PROCESS(Memory);
};





//TODO: create generic connector that takes 2 sc_in_interface references and binds them
// with a sc_signal of whatever type (depending on the ports), the sc_signal should be named
// and added to the vector

#endif