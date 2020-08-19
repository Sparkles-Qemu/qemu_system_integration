#ifndef MEMORY_CPP // Note include guards, this is a quick and dirty way to include components
#define MEMORY_CPP

#include <GlobalControl.cpp>
#include <assert.h>
#include <iostream>
#include <string>
#include <systemc.h>

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
    virtual const sc_vector<sc_signal<DataType>>& mem_read_data() = 0;
    virtual void mem_write_data(const sc_vector<sc_signal<DataType>>& _data) = 0;
    virtual const sc_vector<sc_signal<DataType>>& channel_read_data() = 0;
    virtual void channel_write_data(const sc_vector<sc_signal<DataType>>& _data) = 0;
    virtual void channel_write_data_element(DataType _data, unsigned int col) = 0;
    virtual unsigned int addr() = 0;
    virtual void set_addr(unsigned int addr) = 0;
    virtual bool enabled() = 0;
    virtual void set_enable(bool status) = 0;
    virtual void reset() = 0;
    virtual const unsigned int& get_width() = 0;
};

template <typename DataType>
struct MemoryChannel : public sc_module, public MemoryChannel_IF<DataType>
{
    sc_vector<sc_signal<DataType>> read_channel_data;
    sc_vector<sc_signal<DataType>> write_channel_data;
    sc_signal<unsigned int> channel_addr;
    sc_signal<bool> channel_enabled;
    sc_signal<MemoryChannelMode> channel_mode;
    const unsigned int channel_width;

    MemoryChannel(sc_module_name name, unsigned int width, sc_trace_file* tf) : sc_module(name),
                                                                                read_channel_data("read_channel_data", width),
                                                                                write_channel_data("write_channel_data", width),
                                                                                channel_addr("addr"),
                                                                                channel_enabled("enabled"),
                                                                                channel_mode("mode"),
                                                                                channel_width(width)
    {
        channel_addr = 0;
        channel_enabled = false;
        channel_mode = MemoryChannelMode::READ;

        for (unsigned int i = 0; i < channel_width; i++)
        {
            sc_trace(tf, this->read_channel_data[i], (string(this->read_channel_data[i].name())));
            sc_trace(tf, this->write_channel_data[i], (string(this->write_channel_data[i].name())));
        }

        sc_trace(tf, this->channel_addr, (string(this->channel_addr.name())));
        sc_trace(tf, this->channel_enabled, (string(this->channel_enabled.name())));
        sc_trace(tf, this->channel_mode, (string(this->channel_mode.name())));
    }

    const sc_vector<sc_signal<DataType>>& mem_read_data()
    {
        return write_channel_data;
    }

    void mem_write_data(const sc_vector<sc_signal<DataType>>& _data)
    {
        assert(_data.size() == channel_width);
        for (unsigned int i = 0; i < channel_width; i++)
        {
            read_channel_data[i] = _data[i];
        }
    }

    const sc_vector<sc_signal<DataType>>& channel_read_data()
    {
        return read_channel_data;
    }

    void channel_write_data(const sc_vector<sc_signal<DataType>>& _data)
    {
        assert(_data.size() == channel_width);
        for (unsigned int i = 0; i < channel_width; i++)
        {
            write_channel_data[i] = _data[i];
        }
    }

    void channel_write_data_element(DataType _data, unsigned int col)
    {
        assert(col <= channel_width && col >= 0);
        write_channel_data[col] = _data;
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
        for (auto& data : read_channel_data)
        {
            data = 0;
        }
        for (auto& data : write_channel_data)
        {
            data = 0;
        }
        channel_addr = 0;
        channel_enabled = false;
        channel_mode = MemoryChannelMode::READ;
    }

    const unsigned int& get_width()
    {
        return channel_width;
    }

    void register_port(sc_port_base& port_,
                       const char* if_typename_)
    {
        cout << "now binding    " << port_.name() << " to "
             << "interface: " << if_typename_ << " with channel width " << channel_width << endl;
    }
};

template <typename DataType>
struct MemoryRowCreator
{
    MemoryRowCreator(unsigned int _width, sc_trace_file* _tf) : tf(_tf), width(_width) {}
    sc_vector<sc_signal<DataType>>* operator()(const char* name, size_t)
    {
        return new sc_vector<sc_signal<DataType>>(name, width);
    }
    sc_trace_file* tf;
    unsigned int width;
};

template <typename DataType>
struct MemoryChannelCreator
{
    MemoryChannelCreator(unsigned int _width, sc_trace_file* _tf) : tf(_tf), width(_width) {}
    MemoryChannel<DataType>* operator()(const char* name, size_t)
    {
        return new MemoryChannel<DataType>(name, width, tf);
    }
    sc_trace_file* tf;
    unsigned int width;
};

template <typename DataType>
struct Memory : public sc_module
{
private:
    sc_in_clk _clk;
    // Control Signals
public:
    unsigned int length, width;
    sc_vector<sc_vector<sc_signal<DataType>>> ram;
    sc_port<GlobalControlChannel_IF> control;
    sc_vector<sc_port<MemoryChannel_IF<DataType>>> channels;

    void update()
    {
        if (control->reset())
        {
            for (auto& row : ram)
            {
                for (auto& col : row)
                {
                    col = 0;
                }
            }
        }
        else if (control->enable())
        {
            for (auto& channel : channels)
            {
                if (channel->enabled())
                {
                    switch (channel->mode())
                    {
                    case MemoryChannelMode::WRITE:
                        assert(channel->get_width() == width);
                        for (unsigned int i = 0; i < width; i++)
                        {
                            ram[channel->addr()][i] = channel->mem_read_data()[i];
                        }
                        break;
                    case MemoryChannelMode::READ:
                        assert(channel->get_width() == width);
                        channel->mem_write_data(ram[channel->addr()]);
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
            for (const auto& col : row)
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
        sc_trace_file* tf) : sc_module(name),
                             ram("ram", _length, MemoryRowCreator<DataType>(_width, tf)),
                             control("control"),
                             channels("channel", _channel_count)
    {
        length = _length;
        width = _width;

        for (unsigned int row = 0; row < length; row++)
        {
            for (unsigned int col = 0; col < width; col++)
            {
                sc_trace(tf, ram[row][col], ram[row][col].name());
            }
        }

        control(_control);
        _clk(control->clk());

        SC_METHOD(update);

        sensitive << _clk.pos();
        sensitive << control->reset();

        cout << "MEMORY MODULE: " << name << " has been instantiated " << endl;
    }

    SC_HAS_PROCESS(Memory);
};

//TODO: create generic connector that takes 2 sc_in_interface references and binds them
// with a sc_signal of whatever type (depending on the ports), the sc_signal should be named
// and added to the vector

#endif
