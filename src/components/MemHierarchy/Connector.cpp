#ifndef CONNECTOR_CPP // Note include guards, this is a quick and dirty way to include components
#define CONNECTOR_CPP

#include <systemc.h>
#include <iostream>

using std::cout;
using std::endl;

struct GenericConnector : public sc_module
{
    GenericConnector(sc_module_name name) : sc_module(name)
    {
        std::cout << "CONNECTOR  " << name << " instantiated " << std::endl;
    }
};

template <typename ConnectionType>
struct Connector : public GenericConnector 
{
    //Constructor for module. This module is pos edge trigger
    ConnectionType data;
    Connector(sc_module_name name, sc_module_name connection_name, sc_trace_file *_tf) : GenericConnector(name), data(connection_name)
    {
        
    }

};

#endif