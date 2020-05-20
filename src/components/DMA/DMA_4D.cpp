#ifndef DMA_4D_CPP // Note include guards, this is a quick and dirty way to include components
#define DMA_4D_CPP

#include <systemc.h>
#include "vector"
#include <string>
#include <iostream>
#include "DMA.cpp"

struct Descriptor_4D
{
    unsigned int next;     // index of next descriptor
    unsigned int start;    // start index in ram array
    DmaState state;        // state of dma
    unsigned int x_count;  // number of floats to transfer/wait
    unsigned int x_modify; // number of floats between each transfer/wait
    unsigned int y_count;  // number of floats to transfer/wait
    unsigned int y_modify; // number of floats between each transfer/wait
    unsigned int z_count;  // number of floats to transfer/wait
    unsigned int z_modify; // number of floats between each transfer/wait
    unsigned int k_count;  // number of floats to transfer/wait
    unsigned int k_modify; // number of floats between each transfer/wait
};

struct DMA_4D : public sc_module
{
    // Control Signals
    sc_in<bool> clk, reset, enable;

    // Memory and Stream
    float *ram;
    sc_inout<float> stream;

    // Internal Data
    std::vector<Descriptor_4D> descriptors;
    unsigned int execute_index;
    DmaDirection direction;
    unsigned int current_ram_index;
    unsigned int x_count_remaining;
    unsigned int y_count_remaining;
    unsigned int z_count_remaining;
    unsigned int k_count_remaining;

    const Descriptor_4D default_descriptor = {0, 0, DmaState::SUSPENDED, 0, 0, 0, 0, 0, 0, 0, 0};

    void resetIndexingCounters()
    {
        x_count_remaining = descriptors[execute_index].x_count;
        y_count_remaining = descriptors[execute_index].y_count;
        z_count_remaining = descriptors[execute_index].z_count;
        k_count_remaining = descriptors[execute_index].k_count;    
    }

    void resetAllInternalCounters()
    {
        current_ram_index = descriptors[execute_index].start;
        x_count_remaining = descriptors[execute_index].x_count;
        y_count_remaining = descriptors[execute_index].y_count;
        z_count_remaining = descriptors[execute_index].z_count;
        k_count_remaining = descriptors[execute_index].k_count;    
    }

    void loadProgram(std::vector<Descriptor_4D> newProgram)
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

    bool enabled()
    {
        return enable.read() == true;
    }

    Descriptor_4D currentDescriptor()
    {
        return descriptors[execute_index];
    }

    DmaState getCurrentState()
    {
        return descriptors[execute_index].state;
    }

    void updateCurrentIndex()
    {
        if(x_count_remaining != 0)
        {
            current_ram_index += currentDescriptor().x_modify;
            x_count_remaining--;
        }
        else if (y_count_remaining != 0)
        {
            current_ram_index += currentDescriptor().y_modify;
            x_count_remaining = currentDescriptor().x_count;
            y_count_remaining--;
        }
        else if(z_count_remaining != 0)
        {
            current_ram_index += currentDescriptor().z_modify;
            x_count_remaining = currentDescriptor().x_count;
            y_count_remaining = currentDescriptor().y_count;
            z_count_remaining--;
        }
        else if(k_count_remaining != 0)
        {
            current_ram_index += currentDescriptor().k_modify;
            x_count_remaining = currentDescriptor().x_count;
            y_count_remaining = currentDescriptor().y_count;
            z_count_remaining = currentDescriptor().z_count;
            k_count_remaining--;
        }
        else
        {
            /* DO NOTHING, NEED TO EXECUTE NEXT DESCRIPTOR */
        }
    }

    bool descriptorComplete()
    {
        return (x_count_remaining == 0 && y_count_remaining == 0);
    }

    void loadNextDescriptor()
    {
        execute_index = descriptors[execute_index].next;
        if(currentDescriptor().state == DmaState::TRANSFER_WITH_FORWARD)
        {
            resetIndexingCounters();
        }
        else
        {
            resetAllInternalCounters();
        }
    }

    std::string convertDirectionToString(DmaDirection dir)
    {
        if (dir == DmaDirection::MM2S)
        {
            return "MM2S";
        }
        else
        {
            return "S2MM";
        }
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
        else if (enabled())
        {
            switch (currentDescriptor().state)
            {
            case DmaState::TRANSFER_WITH_FORWARD:
            case DmaState::TRANSFER:
            {
                switch (direction)
                {
                case DmaDirection::MM2S:
                {
                    float dataFromRam = ram[current_ram_index];
                    stream.write(dataFromRam);
                    break;
                }
                case DmaDirection::S2MM:
                {
                    ram[current_ram_index] = stream.read();
                    break;
                }
                }
                updateCurrentIndex();
                if (descriptorComplete())
                {
                    loadNextDescriptor();
                }
                break;
            }
            case DmaState::WAIT:
            {
                updateCurrentIndex();
                if (descriptorComplete())
                {
                    loadNextDescriptor();
                }
                break;
            }
            case DmaState::SUSPENDED:
            {
                if (direction == DmaDirection::MM2S) // clear stream only for MMM2S
                {
                    stream.write(0);
                }
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
    DMA_4D(sc_module_name name, DmaDirection _direction, const sc_signal<bool> &_clk, const sc_signal<bool> &_reset, const sc_signal<bool> &_enable, float *_ram, sc_signal<float, SC_MANY_WRITERS> &_stream) : sc_module(name)
    {
        SC_METHOD(update);
        sensitive << reset;
        sensitive << clk.pos();

        // connect signals
        this->direction = _direction;
        this->clk(_clk);
        this->reset(_reset);
        this->enable(_enable);
        this->ram = _ram;
        this->stream(_stream);

        std::cout << "DMA_4D MODULE: " << name << " has been instantiated " << std::endl;
    }

    // Constructor
    DMA_4D(sc_module_name name) : sc_module(name)
    {
        SC_METHOD(update);
        sensitive << reset;
        sensitive << clk.pos();

        std::cout << "DMA_4D MODULE: " << name << " has been instantiated " << std::endl;
    }

    SC_HAS_PROCESS(DMA_4D);
};

#endif