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
struct Connection : public sc_module
{
    sc_vector<sc_signal<DataType>> signals;
    Connection(sc_module_name name, sc_trace_file* _tf, sc_out<DataType>& out, sc_in<DataType>& in) : sc_module(name),
                                                                                                      signals("signal", 1)
    {
        out.bind(signals[0]);
        in.bind(signals[0]);
        sc_trace(_tf, signals[0], signals[0].name());
        std::cout << "CONNECTION " << name << " instantiated and resolved to one-to-one connection" << std::endl;
    }

    Connection(sc_module_name name, sc_trace_file* _tf, sc_out<DataType>& out, sc_vector<sc_in<DataType>>& in) : sc_module(name),
                                                                                                                 signals("signal", 1)
    {
        out.bind(signals[0]);
        for (unsigned int idx = 0; idx < in.size(); idx++)
        {
            in[idx].bind(signals[0]);
        }
        sc_trace(_tf, signals[0], signals[0].name());
        std::cout << "CONNECTION  " << name << " instantiated and resolved to one-to-many connection" << std::endl;
    }

    Connection(sc_module_name name, sc_trace_file* _tf, sc_vector<sc_out<DataType>>& out, sc_vector<sc_in<DataType>>& in) : sc_module(name),
                                                                                                                            signals("signal", in.size())
    {
        assert(out.size() == in.size());
        for (unsigned int idx = 0; idx < in.size(); idx++)
        {
            in[idx].bind(signals[idx]);
            out[idx].bind(signals[idx]);
            sc_trace(_tf, signals[idx], signals[idx].name());
        }
        std::cout << "CONNECTION " << name << " instantiated and resolved to many-to-many connection" << std::endl;
    }

    void one_to_one_bind(sc_in<DataType>& target)
    {
        target.bind(signals[0]);
    }

    void one_to_many_bind(sc_vector<sc_in<DataType>>& targets)
    {
        for (auto& target : targets)
        {
            target.bind(signals[0]);
        }
    }

    void many_to_many_bind(sc_vector<sc_in<DataType>>& targets)
    {
        for (unsigned int idx = 0; idx < targets.size(); idx++)
        {
            targets[idx].bind(signals[idx]);
        }
    }
};

struct Connector : public sc_module
{
    //Constructor for module. This module is pos edge trigger
    sc_trace_file* tf;
    map<string, unique_ptr<sc_module>> connection_tracker;

    Connector(sc_module_name name, sc_trace_file* _tf) : sc_module(name), tf(_tf)
    {
        std::cout << "CONNECTOR  " << name << " instantiated " << std::endl;
    }

    template <typename DataType>
    void add(sc_module_name name, sc_out<DataType>& out, sc_in<DataType>& in)
    {
        auto old_connection = connection_tracker.find((string)name);
        if (old_connection != connection_tracker.end())
        {
            auto downcast_connection = static_cast<Connection<DataType>*>(old_connection->second.get());
            downcast_connection->one_to_one_bind(in);
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
            auto downcast_connection = static_cast<Connection<DataType>*>(old_connection->second.get());
            downcast_connection->one_to_many_bind(in);
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
            auto downcast_connection = static_cast<Connection<DataType>*>(old_connection->second.get());
            downcast_connection->many_to_many_bind(in);
        }
        else
        {
            connection_tracker[(string)name] = unique_ptr<Connection<DataType>>(new Connection<DataType>(name, tf, out, in));
        }
    }
};

#endif
