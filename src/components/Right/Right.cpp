#ifndef RIGHT_CPP
#define RIGHT_CPP 

#include <systemc.h>
#include <string>
#include <DMA.cpp>
#include <maa.cpp>

SC_MODULE(COMPUTE_BRANCH)
{
	// Stream from MM2S to S2MMs
	sc_in<float> psumIn;
  	sc_in<bool> clk, reset, enable;
	sc_out<float> psumOut;

	sc_signal<float,SC_MANY_WRITERS> pixelInBus;
	sc_vector<sc_signal<float> > interPsumSignals;
	sc_vector<mult_accumulate > peArray;
	unsigned int peArraySize;

	// Memory To Stream
	DMA dma_mm2s;

    void update() {
        if(reset.read() == 1) {
            psumOut.write(0);
        } else if (enable.read() == 1) {
            interPsumSignals[0] = psumIn.read();
            psumOut.write(interPsumSignals[peArraySize+1]);
        }
    }

	// constructor for sc_vector use
	COMPUTE_BRANCH(sc_module_name name) : dma_mm2s("COMPUTE_BRANCH_DMA")	{
		std::cout << "Module: " << name << " COMPUTE_BRANCH has been instantiated with empty constructor" << std::endl;

    	SC_METHOD(update);
	        dont_initialize();
	        sensitive << clk.pos();
	        sensitive << reset;
		
	}

	
	// Constructor with init list of components
	COMPUTE_BRANCH(sc_module_name name, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable, const sc_signal<float>& _psumIn, sc_signal<float>& _psumOut, float* _ram_source, unsigned int arraySize):
		interPsumSignals("interPsumSignals", arraySize+1), // note > peArraySize
		peArray("peArray", arraySize),
		dma_mm2s("external_mm2pixelInBus", DmaDirection::MM2S, _clk, _reset, _enable, _ram_source, pixelInBus)
	{
		psumIn(_psumIn);
		peArraySize = arraySize;

		clk(_clk);
		reset(_reset);
		enable(_enable);

		for(unsigned int i = 0; i<arraySize; i++)
		{
		    peArray[i].pixelIn(pixelInBus);
		    peArray[i].enable(_enable);
		    peArray[i].reset(_reset);
		    peArray[i].clk(_clk);
		    peArray[i].partialSum(interPsumSignals[i]);
		    peArray[i].output(interPsumSignals[i+1]);
		}
		psumOut(_psumOut);
		std::cout << "Module: " << name << " has been instantiated" << std::endl;

    	SC_METHOD(update);
	        dont_initialize();
	        sensitive << clk.pos();
	        sensitive << reset;
		
	}

	SC_HAS_PROCESS(COMPUTE_BRANCH);

};

SC_MODULE(RIGHT)
{
	// Stream from MM2S to S2MMs
  	sc_in<bool> clk, reset, enable;
	sc_out<float> streamOut;
	const unsigned int branchCount = 3;
	sc_vector<sc_signal<float> > interComputeBranchPsum;
	COMPUTE_BRANCH branch0;
	COMPUTE_BRANCH branch1;
	COMPUTE_BRANCH branch2;

    void update() {
        if(reset.read() == 1) {
            streamOut.write(0);
        } else if (enable.read() == 1) {
            interComputeBranchPsum[0] = 0;
            streamOut.write(interComputeBranchPsum[branchCount+1]);
        }
    }

	// Constructor with init list of components
	RIGHT(sc_module_name name, const sc_signal<bool>& _clk, const sc_signal<bool>& _reset, const sc_signal<bool>& _enable, float* _ram_source0, float* _ram_source1, float* _ram_source2, sc_signal<float>& _streamOut) : \
		interComputeBranchPsum("interComputeBranchPsum", branchCount+1),
		branch0("branch0", _clk,  _reset,  _enable, interComputeBranchPsum[0], interComputeBranchPsum[1],  _ram_source0, 3),
		branch1("branch1", _clk,  _reset,  _enable, interComputeBranchPsum[1], interComputeBranchPsum[2],  _ram_source1, 3),
		branch2("branch2", _clk,  _reset,  _enable, interComputeBranchPsum[2], interComputeBranchPsum[3],  _ram_source2, 3)

	{
		clk(_clk);
		reset(_reset);
		enable(_enable);

    	SC_METHOD(update);
        dont_initialize();
        sensitive << clk.pos();
        sensitive << reset;

		std::cout << "Module: " << name << " has been instantiated" << std::endl;
	}
	
	SC_HAS_PROCESS(RIGHT);

};

#endif // LEFT_CPP
