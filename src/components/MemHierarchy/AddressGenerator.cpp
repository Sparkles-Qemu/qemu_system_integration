#ifndef ADDRESS_GENERATOR_CPP // Note include guards, this is a quick and dirty way to include components
#define ADDRESS_GENERATOR_CPP

#include <systemc.h>
#include "map"
#include "vector"
#include <string>
#include <iostream>
#include <assert.h>
#include <GlobalControl.cpp>

using std::cout;
using std::endl;
using std::vector;
using std::string;

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
    // sc_in<bool> clk, reset, enable;
    sc_port<GlobalControlChannel_IF> control;

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
    AddressGenerator(sc_module_name name, const GlobalControlChannel &_control) : sc_module(name), control("control")
    {
        control(_control);

        SC_METHOD(update);
        sensitive << control->clk();
        sensitive << control->reset();

        // connect signals
        std::cout << "ADDRESS_GENERATOR MODULE: " << name << " has been instantiated " << std::endl;
    }

    SC_HAS_PROCESS(AddressGenerator);
};

#endif