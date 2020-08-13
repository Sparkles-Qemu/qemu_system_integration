#ifndef VECTOR_CREATOR_CPP
#define VECTOR_CREATOR_CPP

#include <GlobalControl.cpp>
#include <assert.h>
#include <iostream>
#include <string>
#include <systemc.h>

using std::cout;
using std::endl;
using std::string;

template <typename SyscObject>
struct GenericCreator
{
    GenericCreator(GlobalControlChannel& _control, sc_trace_file* _tf) : tf(_tf), control(_control) {}
    SyscObject* operator()(const char* name, size_t)
    {
        return new SyscObject(name, control, tf);
    }
    sc_trace_file* tf;
    GlobalControlChannel& control;
};

#endif
