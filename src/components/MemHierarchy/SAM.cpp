#ifndef SAM_CPP
#define SAM_CPP

#include <AddressGenerator.cpp>
#include <GlobalControl.cpp>
#include <Memory.cpp>
#include <assert.h>
#include <iostream>
#include <string>
#include <systemc.h>
#include <VectorCreator.cpp>

using std::cout;
using std::endl;
using std::string;

template <typename DataType>
using AddressGeneratorCreator = GenericCreator<AddressGenerator<DataType>>;

template <typename DataType>
struct DataPortCreator
{
    DataPortCreator(unsigned int _width, sc_trace_file* _tf)
        : tf(_tf), width(_width) {}
    sc_vector<DataType>* operator()(const char* name, size_t)
    {
        return new sc_vector<DataType>(name, width);
    }
    sc_trace_file* tf;
    unsigned int width;
};

template <typename DataType>
using InDataPortCreator = DataPortCreator<sc_in<DataType>>;

template <typename DataType>
using OutDataPortCreator = DataPortCreator<sc_out<DataType>>;

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
          mem("mem", _control, _channel_count, _length, _width, tf)

    {
        _clk(control->clk());
        SC_METHOD(update);
        sensitive << _clk.pos();
        sensitive << control->reset();
        cout << " SAM MODULE: " << name << " has been instantiated "
             << endl;
    }

    SC_HAS_PROCESS(SAM);
};
#endif
