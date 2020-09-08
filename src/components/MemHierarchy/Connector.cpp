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

/**
 * @brief Connection object is a wrapper around a sc_vector of signals of
 * arbitrary type. It is used with the connector object to create and bind
 * sc_signals to ports. Each constructor constructs a different connector object
 * based on the type of connection desired. Types of connections are one to one,
 * one to many, and many to many. 
 * 
 * @tparam DataType 
 */
template <typename DataType>
struct Connection : public sc_module
{
    /**
     * @brief vector of signals of arbitrary type defined by the template
     * argument defined upon instantiation of connection objection. 
     */
    sc_vector<sc_signal<DataType>> signals;

    /**
     * @brief Construct a named Connection object to be used in a one to one
     * connection between an output and an input port. The constructor here
     * takes a reference to two ports, instantiates an sc_vector of size 1
     * named "signal" and bind it to the two ports. It also traces said signal
     * automatically using the pointer to the tracefile _tf.
     * 
     * @param name 
     * @param _tf 
     * @param out 
     * @param in 
     */
    Connection(sc_module_name name, sc_trace_file* _tf, sc_out<DataType>& out, sc_in<DataType>& in) : sc_module(name),
                                                                                                      signals("signal", 1)
    {
        out.bind(signals[0]);
        in.bind(signals[0]);
        sc_trace(_tf, signals[0], signals[0].name());
        std::cout << "CONNECTION " << name << " instantiated and resolved to one-to-one connection" << std::endl;
    }

    /**
     * @brief Construct a named Connection object for a one to many connection
     * between one output port and many input ports. The constructor here
     * takes a reference to one output port and a vector of input ports,
     * instantiates an sc_vector of signals of of size 1 named "signal" and
     * binds it to the output port and to each input port. It also traces said
     * signal automatically using the pointer to the tracefile _tf.
     * 
     * @param name 
     * @param _tf 
     * @param out 
     * @param in 
     */
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

    /**
     * @brief Construct a named Connection object for a many to many connection
     * between many output port references and many input port references. The
     * constructor here takes a reference to one output port and a vector of
     * input ports, instantiates an sc_vector of signals of of size determined
     * by the input ports. The signal vector is named "signal". The vector is
     * then bound to the outpiut and input ports. If there is a mismatch between
     * input and output port sizes an exception is thrown and the program
     * terminates. The constructor traces the vector of signals automatically
     * using the pointer to the tracefile _tf.
     *
     * @param name 
     * @param _tf 
     * @param out 
     * @param in 
     */
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

    /**
     * @brief rebinds instantiated sc_vector of signals of size 1 to a new input
     * target
     * 
     * @param target 
     */
    void one_to_one_rebind_to(sc_in<DataType>& target)
    {
        target.bind(signals[0]);
    }

    /**
     * @brief rebinds instantated sc_vector of signals of size 1 to an sc_vector
     * of input targets. 
     * 
     * @param targets 
     */
    void one_to_many_rebind_to(sc_vector<sc_in<DataType>>& targets)
    {
        for (auto& target : targets)
        {
            target.bind(signals[0]);
        }
    }

    /**
     * @brief rebinds instantated sc_vector of signals of to an sc_vector of
     * input targets. Size of sc_vector of signals being bound must be equal to
     * the size of the sc_vector of targets otherwise an exception is thrown and
     * the program is terminates. 
     * 
     * @param targets 
     */
    void many_to_many_rebind_to(sc_vector<sc_in<DataType>>& targets)
    {
        assert(signals.size() == targets.size());
        for (unsigned int idx = 0; idx < targets.size(); idx++)
        {
            targets[idx].bind(signals[idx]);
        }
    }
};

/**
 * @brief Connector object that keeps track of all active signals between ports
 * in modules. Note that ownership of signals is passed to parent of Connector
 * object (Currently not sure why that happens, though it's related to the way
 * systemc resolved naming in the design heirarchy at elaboration). Connectors
 * are not bound to specific DataTypes. They only keep track of pointers to
 * generic sc_modules from which specific templated connections can be downcast
 * depending on which add function is called. Which add function is called is
 * relies on polymorphism to infer which type of connection or "edge" in the
 * heirarchy is required. There are three types of add calls, one to one, one to
 * many, and many to many.
 */
struct Connector : public sc_module
{
    //Constructor for module. This module is pos edge trigger
    sc_trace_file* tf;
    map<string, unique_ptr<sc_module>> connection_tracker;
    map<string, string> output_port_tracker;

    Connector(sc_module_name name, sc_trace_file* _tf) : sc_module(name), tf(_tf)
    {
        std::cout << "CONNECTOR  " << name << " instantiated " << std::endl;
    }

    /**
     * @brief adds a one to one connection between two ports references of type DataType, one output and one
     * input. The connection is given a name and it's type is inferred from the
     * template argument DataType of the ports. The function checks if an older
     * connection exists that can be rebound to a new target input port with the
     * assumption that the target output port already bound to the connection is
     * the same one passed to this function. 
     * 
     * @tparam DataType 
     * @param name 
     * @param out 
     * @param in 
     */
    template <typename DataType>
    void add(sc_module_name name, sc_out<DataType>& out, sc_in<DataType>& in)
    {
        auto old_connection = connection_tracker.find((string)name);
        if (old_connection != connection_tracker.end())
        {
            auto downcasted_connection = dynamic_cast<Connection<DataType>*>(old_connection->second.get());
            assert(downcasted_connection);
            downcasted_connection->one_to_one_rebind_to(in);
            if (output_port_tracker[(string)name] != (string)out.name())
            {
                string error_msg = "Rebinding previously instantated connection that is not currently bound to the output port argument passed named : ";
                error_msg += (string)out.name();
                SC_REPORT_WARNING("", error_msg.c_str());
            }
        }
        else
        {
            connection_tracker[(string)name] = unique_ptr<Connection<DataType>>(new Connection<DataType>(name, tf, out, in));
            output_port_tracker[(string)name] = (string)out.name();
        }
    }

    template <typename DataType>
    void add(sc_module_name name, sc_out<DataType>& out, sc_vector<sc_in<DataType>>& in)
    {
        auto old_connection = connection_tracker.find((string)name);
        if (old_connection != connection_tracker.end())
        {
            auto downcasted_connection = dynamic_cast<Connection<DataType>*>(old_connection->second.get());
            assert(downcasted_connection);
            downcasted_connection->one_to_many_rebind_to(in);
            if (output_port_tracker[(string)name] != (string)out.name())
            {
                string error_msg = "Rebinding previously instantated connection that is not currently bound to the output port argument passed named : ";
                error_msg += (string)out.name();
                SC_REPORT_WARNING("", error_msg.c_str());
            }
        }
        else
        {
            output_port_tracker[(string)name] = (string)out.name();
            connection_tracker[(string)name] = unique_ptr<Connection<DataType>>(new Connection<DataType>(name, tf, out, in));
        }
    }

    template <typename DataType>
    void add(sc_module_name name, sc_vector<sc_out<DataType>>& out, sc_vector<sc_in<DataType>>& in)
    {
        auto old_connection = connection_tracker.find((string)name);
        if (old_connection != connection_tracker.end())
        {
            auto downcasted_connection = dynamic_cast<Connection<DataType>*>(old_connection->second.get());
            assert(downcasted_connection);
            downcasted_connection->many_to_many_rebind_to(in);
            if(output_port_tracker[(string)name] != (string)out.name())
            {
                string error_msg = "Rebinding previously instantated connection that is not currently bound to the output port argument passed named : ";
                error_msg += (string)out.name();
                SC_REPORT_WARNING("", error_msg.c_str());                
            }
        }
        else
        {
            output_port_tracker[(string)name] = (string)out.name();
            connection_tracker[(string)name] = unique_ptr<Connection<DataType>>(new Connection<DataType>(name, tf, out, in));
        }
    }
};

#endif
