#ifndef CONNECTION_CPP // Note include guards, this is a quick and dirty way to include components
#define CONNECTION_CPP

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <systemc.h>

using std::cout;
using std::endl;
using std::map;
using std::string;
using std::unique_ptr;

struct Connection_Base : public sc_module
{
    Connection_Base(sc_module_name name) : sc_module(name)
    {
        std::cout << "BASE CONNECTION " << name << " instantiated " << std::endl;
    }
};

template <typename DataType>
struct Connection : public Connection_Base
{
    sc_vector<sc_signal<DataType>> signals;
    Connection(sc_module_name name, sc_trace_file* _tf, sc_out<DataType>& out, sc_in<DataType>& in) : Connection_Base(name),
                                                                                                      signals("signal", 1)
    {
        out.bind(signals[0]);
        in.bind(signals[0]);
        sc_trace(_tf, signals[0], signals[0].name());
        std::cout << "BASE CONNECTION " << name << " resolved to one-to-one connection" << std::endl;
    }

    Connection(sc_module_name name, sc_trace_file* _tf, sc_out<DataType>& out, sc_vector<sc_in<DataType>>& in) : Connection_Base(name),
                                                                                                                 signals("signal", 1)
    {
        out.bind(signals[0]);
        for (unsigned int idx = 0; idx < in.size(); idx++)
        {
            in[idx].bind(signals[0]);
        }
        sc_trace(_tf, signals[0], signals[0].name());
        std::cout << "BASE CONNECTION  " << name << " resolved to one-to-many connection" << std::endl;
    }

    Connection(sc_module_name name, sc_trace_file* _tf, sc_vector<sc_out<DataType>>& out, sc_vector<sc_in<DataType>>& in) : Connection_Base(name),
                                                                                                                            signals("signal", in.size())
    {
        assert(out.size() == in.size());
        for (unsigned int idx = 0; idx < in.size(); idx++)
        {
            in[idx].bind(signals[idx]);
            out[idx].bind(signals[idx]);
            sc_trace(_tf, signals[idx], signals[idx].name());
        }
        std::cout << "BASE CONNECTION " << name << " resolved to many-to-many connection" << std::endl;
    }
};

struct Connector : public sc_module
{
    //Constructor for module. This module is pos edge trigger
    sc_trace_file* tf;
    map<string, unique_ptr<Connection_Base>> connection_tracker;

    Connector(sc_module_name name, sc_trace_file* _tf) : tf(_tf)
    {
        std::cout << "CONNECTOR  " << name << " instantiated " << std::endl;
    }

    template <typename DataType>
    void add(sc_module_name name, sc_out<DataType>& out, sc_in<DataType>& in)
    {
        auto old_connection = connection_tracker.find((string)name);
        if (old_connection != connection_tracker.end())
        {
            auto connection = static_cast<Connection<DataType>*>(old_connection->second.get());
            in.bind(connection->signals[0]);
        }
        else
        {
            connection_tracker[(string)name] = unique_ptr<Connection<DataType>>(new Connection<DataType>(name, tf, out, in));
        }
    }

    template <typename DataType>
    void add(sc_module_name name, sc_out<DataType>& out, sc_vector<sc_in<DataType>>& in)
    {
        auto old_connection = connection_tracker.find((string)name);
        if (old_connection != connection_tracker.end())
        {
            auto connection = static_cast<Connection<DataType>*>(old_connection->second.get());
            for (unsigned int idx = 0; idx < in.size(); idx++)
            {
                in[idx].bind(connection->signals[0]);
            }
        }
        else
        {
            connection_tracker[(string)name] = unique_ptr<Connection<DataType>>(new Connection<DataType>(name, tf, out, in));
        }
    }

    template <typename DataType>
    void add(sc_module_name name, sc_vector<sc_out<DataType>>& out, sc_vector<sc_in<DataType>>& in)
    {
        auto old_connection = connection_tracker.find((string)name);
        if (old_connection != connection_tracker.end())
        {
            auto connection = static_cast<Connection<DataType>*>(old_connection->second.get());
            for (unsigned int idx = 0; idx < in.size(); idx++)
            {
                in[idx].bind(connection->signals[idx]);
            }
        }
        else
        {
            connection_tracker[(string)name] = unique_ptr<Connection<DataType>>(new Connection<DataType>(name, tf, out, in));
        }
    }
};

#endif
