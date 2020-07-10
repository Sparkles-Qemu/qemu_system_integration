#ifndef MEMORY_CPP // Note include guards, this is a quick and dirty way to include components
#define MEMORY_CPP

#include <systemc.h>
#include "map"
#include "vector"
#include <string>
#include <iostream>

using std::cout;
using std::endl;
using std::vector;
using std::string;

struct DMA_2D : public sc_module
{
    // Control Signals
    sc_in_clk clk;
    sc_in<bool> reset, enable, program;
    
    bool programmed;
    bool realstic_programming_mode;
    bool sram_read_init;

    // Memory and Stream
    float *ram; //TODO: need to change this to allow arbitrary data
    sc_inout<float> stream;

    sc_signal<float> internal_stream;

    // Internal Data
    std::vector<Descriptor_2D> descriptors;

    unsigned int execute_index;
    DmaDirection direction;
    unsigned int current_ram_index;
    unsigned int x_count_remaining;
    unsigned int y_count_remaining;

    unsigned int program_wait_time;
    unsigned int program_forward_time;
    unsigned int program_size;
    unsigned int program_start_index;
    unsigned int program_wait_overhead_time;

    unsigned int default_wait_time;
    unsigned int default_forward_time;
    unsigned int default_program_size;
    unsigned int default_program_wait_overhead_time;
    unsigned int default_program_start_index;
    unsigned int sram_read_delay;

    const Descriptor_2D default_descriptor = {0, 0, DmaState::SUSPENDED, 0, 0, 0, 0};

    // Prints descriptor list, useful for debugging
    void print_descriptors()
    {
        for (uint i = 0; i < descriptors.size(); i++)
        {
            cout << "Descriptor " << i << endl;
            cout << "next: " << descriptors[i].next << endl;
            cout << "start: " << descriptors[i].start << endl;
            cout << "state: " << (int)descriptors[i].state << endl;
            cout << "x_count: " << descriptors[i].x_count << endl;
            cout << "x_modify: " << descriptors[i].x_modify << endl
                 << endl;
            cout << "y_count: " << descriptors[i].y_count << endl;
            cout << "y_modify: " << descriptors[i].y_modify << endl
                 << endl;
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
        if (currentDescriptor().state == DmaState::TRANSFER_WITH_COUNTER_FORWARD)
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

    bool programmingMode()
    {
        return program.read() == true;
    }

    void relative_wait(unsigned int time)
    {
        if(time != 0)
        {
            wait(time);
        }
    }

    void parseMetaData(const vector<unsigned int> &received_data,
                        unsigned int &wait_time,
                        unsigned int &forward_time,
                        unsigned int &program_size,
                        unsigned int &wait_overhead_time,
                        unsigned int &program_start_index)
    {
        wait_time = received_data[0];
        forward_time = received_data[1];
        program_size = received_data[2];
        wait_overhead_time = received_data[3];
        program_start_index = received_data[4];
    }

    vector<Descriptor_2D> parseProgramData(vector<unsigned int>& received_data)
    {
        // TODO: implement parsing that allows arbitrary descriptor sizes
        vector<Descriptor_2D> program_data;
        char* raw_received_data = (char*)(received_data.data());
        for(unsigned int i = 0; i<received_data.size()*sizeof(unsigned int); i+=sizeof(Descriptor_2D))
        {
            Descriptor_2D desc = *((Descriptor_2D*)(raw_received_data+i));
            program_data.push_back(desc);
        }
        return program_data; 
    }

    vector<unsigned int> runProgrammingSequence(unsigned int wait_time, unsigned int forward_time, unsigned int program_size, unsigned int wait_overhead_time, unsigned int program_start_index)
    {
        sram_read_init = true;

        cout << "@ " << sc_time_stamp() << " " << this->name() << ": DMA MODULE is waiting for data from parent node" << endl;

        if(wait_time != 0)
        {
            wait(wait_time); //relative to last clk rising edge
        }

        cout << "@ " << sc_time_stamp() << " " << this->name() << ": DMA MODULE is forwarding all data to child nodes" << endl;
        
        unsigned int start_index = program_start_index;
        for (unsigned int i = 0; i < forward_time; i++)
        {
            switch (direction)
            {
            case DmaDirection::MM2S:
            {
                if (sram_read_init)
                {
                    wait(sram_read_delay);
                    sram_read_init = false;
                }
                float dataFromRam = ram[start_index++];
                stream.write(dataFromRam);
                break;
            }
            case DmaDirection::S2MM:
            {
                ram[start_index++] = internal_stream;
                break;
            }
            }
            wait();
        }

        cout << "@ " << sc_time_stamp() << " " << this->name() << ": DMA MODULE is saving all relevant data from parent node" << endl;

        vector<unsigned int> received_data;
        for (unsigned int i = 0; i < program_size; i++)
        {
            switch (direction)
            {
            case DmaDirection::MM2S:
            {
                float dataFromRam = ram[start_index++];
                received_data.push_back(dataFromRam);
                break;
            }
            case DmaDirection::S2MM:
            {
                received_data.push_back(internal_stream);
                break;
            }
            }
            wait();
        }

        cout << "@ " << sc_time_stamp() << " " << this->name() << " : DMA MODULE is waiting for all other nodes to be ready" << endl;

        if(wait_overhead_time != 0)
        {
            wait(wait_overhead_time);
        }

        cout << "@ " << sc_time_stamp() << " " << this->name() << " : DMA MODULE is ready" << endl;

        return received_data;
    }

    void loadProgramS() // TODO: Change name after deprecating magic loadProgram
    {
        while (true)
        {
            if (programmingMode() && !programmed) 
            {
                programmed = false;
        
                vector<unsigned int> received_data;
                cout << "@ " << sc_time_stamp() << " " << this->name() << " : DMA MODULE has entered programming phase" << endl;
                cout << "@ " << sc_time_stamp() << " " << this->name() << " : DMA MODULE is fetching metadata" << endl;

                received_data = runProgrammingSequence(
                    default_wait_time,
                    default_forward_time,
                    default_program_size,
                    default_program_wait_overhead_time,
                    default_program_start_index);

                parseMetaData(received_data,
                                program_wait_time,
                                program_forward_time,
                                program_size,
                                program_wait_overhead_time,
                                program_start_index);

                cout << "@ " << sc_time_stamp() << " " << this->name() << " : DMA MODULE has acquired \n" \
                << "Program wait time: " << program_wait_time << endl \
                << "Program forward time: " << program_forward_time << endl \
                << "Program Size: " << program_size << endl \
                << "Program Wait Overhead: " << program_wait_overhead_time << endl \
                << "Program Start Index: " << program_start_index << endl;

                // cout << "@ " << sc_time_stamp() << " " << this->name() << " : DMA MODULE is fetching program data" << endl;

                // received_data = runProgrammingSequence(
                //     program_wait_time,
                //     program_forward_time,
                //     program_size,
                //     program_wait_overhead_time,
                //     program_start_index);

                // descriptors.clear();
                // descriptors = parseProgramData(received_data);

                programmed = true;
                // execute_index = 0;
                // resetAllInternalCounters();
            }
            wait();
        }
    }

    // Called on rising edge of clk or high level reset
    void run()
    {
        if (reset.read())
        {
            resetProgramMemory();
            resetAllInternalCounters();
            programmed = (realstic_programming_mode)? false : true; // TODO: need to change this to force programming mode
            cout << "@ " << sc_time_stamp() << " " << this->name() << ": MODULE has been reset" << endl;
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
                    ram[current_ram_index] = stream.read(); //TODO: update this to internal_signal to eliminate implicit register
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
                cout << "@ " << sc_time_stamp() << " " << this->name() << ": Is in an invalid state! ... exitting" << endl;
                exit(-1);
            }
            }
        }
    }

    void update_inputs()
    {
        if(direction == DmaDirection::S2MM)
        {
            internal_stream = stream.read();
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
        unsigned int _default_program_wait_overhead_time,
        unsigned int _default_program_index,
        unsigned int _sram_read_delay,
        bool _realstic_programming_mode,
        sc_trace_file* tf) : sc_module(name)
    {

        this->realstic_programming_mode = _realstic_programming_mode;

        SC_METHOD(run);
        sensitive << reset;
        sensitive << clk.pos();

        if(_realstic_programming_mode)
        {
            SC_THREAD(loadProgramS); //TODO: remember to change this after removing magic load
            sensitive << clk.pos();
            dont_initialize();
        }

        SC_METHOD(update_inputs);
        sensitive << stream;

        this->default_wait_time = _default_wait_time;
        this->default_forward_time = _default_forward_time;
        this->default_program_size = _default_program_size;
        this->sram_read_delay = _sram_read_delay;
        this->default_program_wait_overhead_time = _default_program_wait_overhead_time;

        // connect signals
        this->direction = _direction;
        this->clk(_clk);
        this->reset(_reset);
        this->enable(_enable);
        this->program(_program_mode);
        this->ram = _ram;
        this->stream(_stream);
 
        string clk_name = (string(this->name()) + string(".clk"));
        string reset_name = (string(this->name()) + string(".reset"));
        string enable_name = (string(this->name()) + string(".enable"));
        string program_name = (string(this->name()) + string(".program"));
        string ram_name = (string(this->name()) + string(".ram"));
        string stream_name = (string(this->name()) + string(".stream"));
        string internal_stream_name = (string(this->name()) + string(".internal_stream"));

        sc_trace(tf, this->clk, clk_name);
        sc_trace(tf, this->reset, reset_name);
        sc_trace(tf, this->enable, enable_name);
        sc_trace(tf, this->program, program_name);
        sc_trace(tf, this->ram, ram_name);
        sc_trace(tf, this->stream, stream_name);
        sc_trace(tf, this->internal_stream, internal_stream_name);

        cout << "DMA_2D MODULE: " << name << " has been instantiated " << endl;
    }

    // Constructor
    DMA_2D(sc_module_name name) : sc_module(name)
    {
        SC_METHOD(run);
        sensitive << reset;
        sensitive << clk.pos();

        SC_THREAD(loadProgramS);
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_inputs);
        sensitive << stream;

        cout << "DMA_2D MODULE: " << name << " has been instantiated " << endl;
    }

    SC_HAS_PROCESS(DMA_2D);
};

#endif