#ifndef DMA_CPP // Note include guards, this is a quick and dirty way to include components
#define DMA_CPP

#include <systemc.h>
#include "map"
#include "modelutil.cpp"
#include "vector"
#include <string>
#include <iostream>

//-----------------------------------------------------
// Design Name : DMA
// File Name   : DMA.cpp
// Function    : Direct Memory Access
// Coder       : Jacob Londa and Owen Beringer
//-----------------------------------------------------

enum class DmaDirection
{
  MM2S,
  S2MM
};

enum class DmaState
{
  SUSPENDED, // do nothing indefinitely
  TRANSFER,  // transfer data
  WAIT       // do nothing for certain number of cycles
};

struct Descriptor
{
  unsigned int next;     // index of next descriptor
  unsigned int start;    // start index in ram array
  DmaState state;        // state of dma
  unsigned int x_count;  // number of floats to transfer/wait
  unsigned int x_modify; // number of floats between each transfer/wait
};

// DMA module definition for MM2S and S2MM
struct DMA : public sc_module
{

  // Control Signals
  sc_in<bool> clk, reset, enable;

  // Memory and Stream
  float *ram;
  sc_inout<float> stream;

  // Internal Data
  std::vector<Descriptor> descriptors;
  unsigned int execute_index;
  DmaDirection direction;
  unsigned int current_ram_index;
  unsigned int x_count_remaining;

  const Descriptor default_descriptor = {0, 0, DmaState::SUSPENDED, 0, 0};

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
    }
  }

  void loadProgram(std::vector<Descriptor> newProgram)
  {
    descriptors.clear();
    for (unsigned int i = 0; i < newProgram.size(); i++)
    {
      descriptors.push_back(newProgram[i]);
    }
    execute_index = 0;
    current_ram_index = descriptors[execute_index].start;
    x_count_remaining = descriptors[execute_index].x_count;
  }

  // Called on rising edge of clk or high level reset
  void update()
  {
    if (reset.read())
    {
      // assume at least one descriptor is in dma at all times
      execute_index = 0;
      descriptors.clear();
      descriptors.push_back(default_descriptor);
      current_ram_index = descriptors[execute_index].start;
      x_count_remaining = descriptors[execute_index].x_count;
      descriptors[execute_index].state = DmaState::SUSPENDED; // slightly cheating here, but does what we want
      std::cout << "@ " << sc_time_stamp() << " " << this->name() << ": Module has been reset" << std::endl;
    }
    else if (enable.read() && (descriptors[execute_index].state != DmaState::SUSPENDED))
    {
      if (descriptors[execute_index].state == DmaState::TRANSFER)
      {
        if (direction == DmaDirection::MM2S) // Memory to Stream
        {
          float value = *(ram + current_ram_index);
          // std::cout << "@ " << sc_time_stamp() << " " << this->name() << " desc " << execute_index << ": Transfering [" << value << "] from RAM to stream" << std::endl;
          stream.write(value);
        }
        else // Stream to Memory
        {
          // std::cout << "@ " << sc_time_stamp() << " " << this->name() << " desc " << execute_index << ": Transfering [" << stream.read() << "] from stream to RAM" << std::endl;
          *(ram + current_ram_index) = stream.read();
        }

        // update ram index
        current_ram_index += descriptors[execute_index].x_modify;
      }
      else // Waiting state
      {
        if (direction == DmaDirection::MM2S) // clear stream only for MMM2S
          stream.write(0);
        // std::cout << "@ " << sc_time_stamp() << " " << this->name() << " desc " << execute_index << ": Waiting..." << std::endl;
      }

      x_count_remaining--;

      if (x_count_remaining == (unsigned int)0) // descriptor is finished, load next descriptor
      {
        execute_index = descriptors[execute_index].next;
        current_ram_index = descriptors[execute_index].start;
        x_count_remaining = descriptors[execute_index].x_count;
      }
    }
    else // Suspended state
    {
      if (direction == DmaDirection::MM2S) // clear stream only for MMM2S
        stream.write(0);
    }
  }

  // Constructor
  DMA(sc_module_name name, DmaDirection _direction, const sc_signal<bool> &_clk, const sc_signal<bool> &_reset, const sc_signal<bool> &_enable, float *_ram, sc_signal<float, SC_MANY_WRITERS> &_stream) : sc_module(name)
  {
    // std::cout << "DMA Module: " << name << " attempting to instantiate " << std::endl;

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

    std::cout << "DMA Module: " << name << " has been instantiated " << std::endl;
  }

  // Constructor
  DMA(sc_module_name name) : sc_module(name)
  {
    // std::cout << "DMA Module: " << name << " attempting to instantiate " << std::endl;

    SC_METHOD(update);
    sensitive << reset;
    sensitive << clk.pos();

    std::cout << "DMA Module: " << name << " has been instantiated with empty constructor" << std::endl;
  }

  SC_HAS_PROCESS(DMA);
};

// DMA module definition for MM2MM
struct DMA_MM2MM : public sc_module
{
  // stream interconnect
  sc_signal<float, SC_MANY_WRITERS> stream;

  // DMA devices
  DMA mm2s, s2mm;

  // Constructor
  DMA_MM2MM(sc_core::sc_module_name name, const sc_signal<bool> &_clk, const sc_signal<bool> &_reset, const sc_signal<bool> &_enable, float *_ram_source, float *_ram_destination) : sc_module(name),
                                                                                                                                                                                     mm2s("internal_mm2s", DmaDirection::MM2S, _clk, _reset, _enable, _ram_source, stream),
                                                                                                                                                                                     s2mm("internal_s2mm", DmaDirection::S2MM, _clk, _reset, _enable, _ram_destination, stream)
  {
    std::cout << "Module: " << name << " has been instantiated" << std::endl;
  }

  SC_HAS_PROCESS(DMA_MM2MM);
};

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

struct DMA_2D : public sc_module
{
  // Control Signals
  sc_in<bool> clk, reset, enable;

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

  void resetInternalCounters()
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
    resetInternalCounters();
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
    resetInternalCounters();
  }

  std::string convertDirectionToString(DmaDirection dir)
  {
    if(dir == DmaDirection::MM2S)
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
      resetInternalCounters();
      std::cout << "@ " << sc_time_stamp() << " " << this->name() << ":MODULE has been reset" << std::endl;
    }
    else if (enabled())
    {
      switch (currentDescriptor().state)
      {
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
  DMA_2D(sc_module_name name, DmaDirection _direction, const sc_signal<bool> &_clk, const sc_signal<bool> &_reset, const sc_signal<bool> &_enable, float *_ram, sc_signal<float, SC_MANY_WRITERS> &_stream) : sc_module(name)
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

#endif // DMA_CPP
