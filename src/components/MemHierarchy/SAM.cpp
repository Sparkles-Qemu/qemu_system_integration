#ifndef SAM_CPP
#define SAM_CPP

#include "sysc/communication/sc_signal_ports.h"
#include <AddressGenerator.cpp>
#include <GlobalControl.cpp>
#include <Memory.cpp>
#include <assert.h>
#include <iostream>
#include <string>
#include <systemc.h>

using std::cout;
using std::endl;
using std::string;

template <typename DataType>
struct SAMDataPortCreator
{
    SAMDataPortCreator(unsigned int _width, sc_trace_file* _tf)
        : tf(_tf), width(_width) {}
    sc_vector<DataType>* operator()(const char* name, size_t)
    {
        return new sc_vector<DataType>(name, width);
    }
    sc_trace_file* tf;
    unsigned int width;
};

template <typename DataType>
using InDataPortCreator = SAMDataPortCreator<sc_in<DataType>>;

template <typename DataType>
using OutDataPortCreator = SAMDataPortCreator<sc_out<DataType>>;

template <typename DataType>
struct SAM : public sc_module
{
    // Member Signals
private:
    sc_in_clk _clk;

public:
    sc_port<GlobalControlChannel_IF> control;
    Memory<DataType> mem;
    sc_vector<AddressGenerator<DataType>> generators;
    sc_vector<MemoryChannel<DataType>> channels;
    sc_vector<sc_vector<sc_in<DataType>>> read_channel_data;
    sc_vector<sc_vector<sc_out<DataType>>> write_channel_data;
    const unsigned int length, width, channel_count;

    void update()
    {
        if (control->reset())
        {
        }
        else if (control->enable())
        {
        }
    }

    // Constructor
    SAM(sc_module_name name, GlobalControlChannel& _control,
        unsigned int _channel_count, unsigned int _length,
        unsigned int _width, sc_trace_file* tf)
        : sc_module(name),
          mem("mem", _control, _channel_count, _length, _width, tf),
          generators("generator", _channel_count, AddressGeneratorCreator<DataType>(_control, tf)),
          channels("mem_channels", _channel_count, MemoryChannelCreator<DataType>(_width, tf)),
          read_channel_data("read_channel_data", _channel_count, InDataPortCreator<DataType>(_width, tf)),
          write_channel_data("write_channel_data", _channel_count, OutDataPortCreator<DataType>(_width, tf)),
          length(_length),
          width(_width),
          channel_count(_channel_count)
    {
        control(_control);
        _clk(control->clk());
        SC_METHOD(update);
        sensitive << _clk.pos();
        sensitive << control->reset();

        for (unsigned int channel_index = 0; channel_index < channel_count; channel_index++)
        {
            generators[channel_index].channel(channels.at(channel_index));
            mem.channels[channel_index](channels.at(channel_index));
            for (unsigned int data_index = 0; data_index < width; data_index++)
            {
                read_channel_data[channel_index][data_index](channels[channel_index].get_channel_read_data_bus()[data_index]);
                write_channel_data[channel_index][data_index](channels[channel_index].get_channel_write_data_bus()[data_index]);

                sc_trace(tf, read_channel_data[channel_index][data_index], read_channel_data[channel_index][data_index].name());
                sc_trace(tf, write_channel_data[channel_index][data_index], write_channel_data[channel_index][data_index].name());
            }
        }
        cout << " SAM MODULE: " << name << " has been instantiated "
             << endl;
    }

    SC_HAS_PROCESS(SAM);
};
#endif
