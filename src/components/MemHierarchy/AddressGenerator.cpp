#ifndef ADDRESS_GENERATOR_CPP
#define ADDRESS_GENERATOR_CPP

#include "map"
#include "vector"
#include <GlobalControl.cpp>
#include <Memory.cpp>
#include <VectorCreator.cpp>
#include <assert.h>
#include <iostream>
#include <string>
#include <systemc.h>

using std::cout;
using std::endl;
using std::string;
using std::vector;

// template <typename Type>
// struct GenericCreator
// {
//     GenericCreator(sc_trace_file* _tf) : tf(_tf) {}
//     Type* operator()(const char* name, size_t) { return new Type(name, tf); }
//     sc_trace_file* tf;
// };

struct GenericControlBus : public sc_module
{
    sc_signal<bool> clk, reset, enable;
    GenericControlBus(sc_module_name name, sc_trace_file* tf)
        : sc_module(name), clk("clk"), reset("reset"), enable("enable")
    {
        sc_trace(tf, this->clk, (string(this->clk.name())));
        sc_trace(tf, this->reset, (string(this->reset.name())));
        sc_trace(tf, this->enable, (string(this->enable.name())));

        cout << "GENERIC CONTROL BUS MODULE: " << name << " has been instantiated "
             << endl;
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
    int x_modify;          // number of floats between each transfer/wait
    unsigned int y_count;  // number of floats to transfer/wait
    int y_modify;          // number of floats between each transfer/wait

    Descriptor_2D(unsigned int _next, unsigned int _start, DescriptorState _state,
                  unsigned int _x_count, int _x_modify, unsigned int _y_count,
                  int _y_modify)
    {
        this->next = _next;
        this->start = _start;
        this->state = _state;
        this->x_count = _x_count;
        this->x_modify = _x_modify;
        this->y_count = _y_count;
        this->y_modify = _y_modify;
    }

    bool operator==(const Descriptor_2D& rhs)
    {
        return this->next == rhs.next && this->start == rhs.start &&
               this->state == rhs.state && this->x_count == rhs.x_count &&
               this->x_modify == rhs.x_modify && this->y_count == rhs.y_count &&
               this->y_modify == rhs.y_modify;
    }
};

Descriptor_2D default_descriptor = {0, 0, DescriptorState::SUSPENDED, 0, 0,
                                    0, 0};




template <typename DataType>
struct AddressGenerator : public sc_module
{
    // Control Signals
private:
    sc_in_clk _clk;
    sc_in<bool> _reset;

public:
    sc_port<GlobalControlChannel_IF> control;
    sc_port<MemoryChannel_IF<DataType>> channel;
    sc_trace_file* tf;

    // Internal Data
    vector<Descriptor_2D> descriptors;
    sc_signal<unsigned int> execute_index;
    sc_signal<unsigned int> current_ram_index;
    sc_signal<unsigned int> x_count_remaining;
    sc_signal<unsigned int> y_count_remaining;
    sc_signal<bool> programmed;
    sc_signal<bool> first_cycle;

    void resetIndexingCounters()
    {
        x_count_remaining = descriptors[execute_index].x_count;
        y_count_remaining = descriptors[execute_index].y_count;
    }

    void loadInternalCountersFromIndex(unsigned int index)
    {
        current_ram_index = descriptors.at(index).start;
        x_count_remaining = descriptors.at(index).x_count;
        y_count_remaining = descriptors.at(index).y_count;
    }

    void loadProgram(const vector<Descriptor_2D>& newProgram)
    {
        descriptors.clear();
        copy(newProgram.begin(), newProgram.end(), std::back_inserter(descriptors));
    }

    void resetProgramMemory()
    {
        execute_index = 0;
        descriptors.clear();
        descriptors.push_back(default_descriptor);
    }

    Descriptor_2D currentDescriptor() { return descriptors.at(execute_index); }
    Descriptor_2D nextDescriptor() { return descriptors.at(descriptors[execute_index].next); }

    void updateCurrentIndex()
    {
        if (x_count_remaining != 0)
        {
            x_count_remaining = x_count_remaining - 1;
        }

        if (x_count_remaining == 0)
        {
            if (y_count_remaining != 0)
            {
                current_ram_index = current_ram_index + currentDescriptor().y_modify;
                // HACK WITH CHANNEL->SET_ADDR
                channel->set_addr(current_ram_index + currentDescriptor().y_modify);
                x_count_remaining = currentDescriptor().x_count;
                y_count_remaining = y_count_remaining - 1;
            }
        }
        else
        {
            // HACK WITH CHANNEL->SET_ADDR
            current_ram_index = current_ram_index + currentDescriptor().x_modify;
            channel->set_addr(current_ram_index + currentDescriptor().x_modify);
        }
    }

    bool descriptorComplete()
    {
        return (x_count_remaining == 0 && y_count_remaining == 0);
    }

    void loadNextDescriptor()
    {
        execute_index = currentDescriptor().next;
        loadInternalCountersFromIndex(currentDescriptor().next);
        channel->set_addr(nextDescriptor().start);
    }

    void update()
    {
        if (control->reset())
        {
            resetProgramMemory();
            loadInternalCountersFromIndex(0);
            programmed = false;
            first_cycle = false;
            channel->reset();
            std::cout << "@ " << sc_time_stamp() << " " << this->name()
                      << ":MODULE has been reset" << std::endl;
        }
        else if (control->program())
        {
            // TODO: Extend with programming logic
            execute_index = 0;
            loadInternalCountersFromIndex(0);
            channel->set_addr(descriptors.at(0).start);
            programmed = true;
            first_cycle = true;
            std::cout << "@ " << sc_time_stamp() << " " << this->name()
                << ":MODULE has been programmed" << std::endl;
        }
        else if (control->enable() && programmed)
        {
            // Update internal logic
            if (!first_cycle && (currentDescriptor().state == DescriptorState::GENERATE ||
                currentDescriptor().state == DescriptorState::WAIT))
            {
                updateCurrentIndex();
                if (descriptorComplete())
                {
                    loadNextDescriptor();
                }
            }
            else
            {
                first_cycle = false;
            }
            
            // update external signals NOTE SEE HACK WITH CHANNEL->SET_ADDR
            if(!descriptorComplete())
            {
                switch (currentDescriptor().state)
                {
                case DescriptorState::GENERATE:
                {
                    channel->set_enable(true);
                    break;
                }
                case DescriptorState::WAIT:
                case DescriptorState::SUSPENDED:
                {
                    channel->set_enable(false);
                    break;
                }
                default:
                {
                    std::cout << "@ " << sc_time_stamp() << " " << this->name()
                            << ": Is in an invalid state! ... exitting" << std::endl;
                    exit(-1);
                }
                }
            }
            else
            {
                // pre enable for next descriptors first cycle
                switch (nextDescriptor().state)
                {
                case DescriptorState::GENERATE:
                {
                    channel->set_enable(true);
                    break;
                }
                case DescriptorState::WAIT:
                case DescriptorState::SUSPENDED:
                {
                    channel->set_enable(false);
                    break;
                }
                default:
                {
                    std::cout << "@ " << sc_time_stamp() << " " << this->name()
                            << ": Is in an invalid state! ... exitting" << std::endl;
                    exit(-1);
                }
                }
            }
            

        }
    }

    // Constructor
    AddressGenerator(sc_module_name name, GlobalControlChannel& _control,
                     sc_trace_file* _tf)
        : sc_module(name), control("control"), channel("channel"), tf(_tf),
          execute_index("execute_index"),
          current_ram_index("current_ram_index"),
          x_count_remaining("x_count_remaining"),
          y_count_remaining("y_count_remaining")
    {
        control(_control);
        _clk(control->clk());
        _reset(control->reset());
        execute_index = 0;
        // sc_trace(tf, this->execute_index, (this->execute_index.name()));
        sc_trace(tf, this->execute_index, (this->execute_index.name()));
        sc_trace(tf, this->current_ram_index, (this->current_ram_index.name()));
        sc_trace(tf, this->x_count_remaining, (this->x_count_remaining.name()));
        sc_trace(tf, this->y_count_remaining, (this->y_count_remaining.name()));

        SC_METHOD(update);
        sensitive << _clk.pos();
        sensitive << _reset.pos();

        // connect signals
        std::cout << "ADDRESS_GENERATOR MODULE: " << name
                  << " has been instantiated " << std::endl;
    }

    SC_HAS_PROCESS(AddressGenerator);
};

template <typename DataType>
using AddressGeneratorCreator = GenericCreator<AddressGenerator<DataType>>;

#endif
