#ifndef DMA_2D_CPP // Note include guards, this is a quick and dirty way to include components
#define DMA_2D_CPP

#include <systemc.h>
#include "map"
#include "vector"
#include <string>
#include <iostream>
#include "DMA.cpp"

struct Descriptor_2D
{
    unsigned int next;     // index of next descriptor
    unsigned int start;    // start index in ram array
    DmaState state;        // state of dma
    unsigned int x_count;  // number of floats to transfer/wait
    unsigned int x_modify; // number of floats between each transfer/wait
    unsigned int y_count;  // number of floats to transfer/wait
    unsigned int y_modify; // number of floats between each transfer/wait
};

enum class DMA_Program_Phase
{
    PHASE_1_WAIT, 
    PHASE_1_FORWARD,
    PHASE_1_SAVE,  
    PHASE_2_WAIT, 
    PHASE_2_FORWARD,
    PHASE_2_SAVE 
};


struct DMA_2D : public sc_module
{
    // Control Signals
    sc_in<bool> clk, reset, enable, program_mode;
    bool programmed;
    bool sram_read_init;

    // Memory and Stream
    float *ram;
    sc_inout<float> stream;

    // Internal Data
    std::vector<Descriptor_2D> descriptors;
    unsigned int execute_index;
    DmaDirection direction;
    unsigned int current_ram_index;
    unsigned int x_count_remaining;
    unsigned int y_count_remaining;
    unsigned int program_wait_time;
    unsigned int program_forward_time;
    unsigned int program_program_size;
    unsigned int default_wait_time;
    unsigned int default_forward_time;
    unsigned int default_program_size;
    unsigned int default_program_index;
    unsigned int sram_read_delay;
    DMA_Program_Phase program_phase;

    const Descriptor_2D default_descriptor = {0, 0, DmaState::SUSPENDED, 0, 0, 0, 0};

    // Prints descriptor list, useful for debugging
    void print_descriptors()
    {
        for (uint i = 0; i < descriptors.size(); i++)
        {
            std::cout << "Descriptor " << i << std::endl;
            std::cout << "next: " << descriptors[i].next << std::endl;
            std::cout << "start: " << descriptors[i].start << std::endl;
            std::cout << "state: " << (int)descriptors[i].state << std::endl;
            std::cout << "x_count: " << descriptors[i].x_count << std::endl;
            std::cout << "x_modify: " << descriptors[i].x_modify << std::endl
                      << std::endl;
            std::cout << "y_count: " << descriptors[i].y_count << std::endl;
            std::cout << "y_modify: " << descriptors[i].y_modify << std::endl
                      << std::endl;
        }
    }

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

    void loadProgram(std::vector<Descriptor_2D> newProgram)
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

    Descriptor_2D currentDescriptor()
    {
        return descriptors[execute_index];
    }

    DmaState getCurrentState()
    {
        return descriptors[execute_index].state;
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
        if(currentDescriptor().state == DmaState::TRANSFER_WITH_COUNTER_FORWARD)
        {
            resetIndexingCounters();
        }
        else
        {
            resetAllInternalCounters();
        }
        sram_read_init = true; // new descriptor, likely at different starting point (otherwise why bother with new descriptor)
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

    bool programMode()
    {
        return program_mode.read() == true;
    }

    // Called on rising edge of clk or high level reset
    void update()
    {
        while(true)
        {
            if (reset.read())
            {
                resetProgramMemory();
                resetAllInternalCounters();
                program_phase = DMA_Program_Phase::PHASE_1_WAIT;
                programmed = true; // TODO: need to change this
                sram_read_init = true;
                std::cout << "@ " << sc_time_stamp() << " " << this->name() << ": MODULE has been reset" << std::endl;
                wait();
            }
            else if (programMode() && !programmed)
            {
                switch(program_phase)
                {
                    case DMA_Program_Phase::PHASE_1_WAIT:
                    {
                        wait(default_wait_time);
                        break;
                    }
                    case DMA_Program_Phase::PHASE_1_FORWARD:
                    {
                        switch(direction)
                        {
                            case DmaDirection::MM2S:
                            {
                                if(sram_read_init)
                                {
                                    wait(sram_read_delay);
                                    sram_read_delay = false;
                                }
                                else
                                {
                                    float dataFromRam = ram[default_program_index];
                                    stream.write(dataFromRam);
                                }
                                break;
                            }
                            case DmaDirection::S2MM:
                            {
                                ram[default_program_index] = stream.read();
                                break;
                            }
                        }
                    }
                    // TODO: Implement rest of programming phases
                    default:
                    break;
                }
            }
            else if (enabled() && programmed)
            {
                switch (currentDescriptor().state)
                {
                case DmaState::TRANSFER_WITH_COUNTER_FORWARD:
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
                wait();
            }
            else
            {
                wait();
            }            
        }
    }

    // Constructor
    DMA_2D(
            sc_module_name name, 
            DmaDirection _direction, 
            const sc_signal<bool> &_clk, 
            const sc_signal<bool> &_reset, 
            const sc_signal<bool> &_enable, 
            const sc_signal<bool> &_program_mode, 
            float *_ram, 
            sc_signal<float, SC_MANY_WRITERS> &_stream,
            unsigned int _default_wait_time,
            unsigned int _default_forward_time,
            unsigned int _default_program_size,
            unsigned int _default_program_index,
            unsigned int _sram_read_delay
        ) : sc_module(name)
    {
        SC_THREAD(update);
        sensitive << reset;
        sensitive << clk.pos();
        dont_initialize();

        this->default_wait_time = _default_wait_time;
        this->default_forward_time = _default_forward_time;
        this->default_program_size = _default_program_size;
        this->sram_read_delay = _sram_read_delay;

        // connect signals
        this->direction = _direction;
        this->clk(_clk);
        this->reset(_reset);
        this->enable(_enable);
        this->program_mode(_program_mode);
        this->ram = _ram;
        this->stream(_stream);

        std::cout << "DMA_2D MODULE: " << name << " has been instantiated " << std::endl;
    }

    // Constructor
    DMA_2D(sc_module_name name) : sc_module(name)
    {
        SC_METHOD(update);
        sensitive << reset;
        sensitive << clk.pos();

        std::cout << "DMA_2D MODULE: " << name << " has been instantiated " << std::endl;
    }

    SC_HAS_PROCESS(DMA_2D);
};

#endif