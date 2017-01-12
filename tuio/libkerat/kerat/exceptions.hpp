/**
 * \file      exceptions.hpp
 * \brief     Contains the declarations of all the exceptions used in this library.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-11-13 00:20 UTC+2
 * \copyright BSD
 */


#ifndef KERAT_EXCEPTIONS_HPP
#define KERAT_EXCEPTIONS_HPP

#include <exception>
#include <stdexcept>
#include <string>

namespace libkerat {
    namespace exception {

        //! \brief Exception class that signalizes an error during network comunication setup
        class net_setup_error: public std::logic_error {
        public:
            explicit net_setup_error(const std::string & reason) throw ()
                :std::logic_error(reason){ ; }
        };

        //! \brief Generic graph error
        class graph_error: public std::logic_error {
        public:
            explicit graph_error(const std::string & reason) throw ()
                :std::logic_error(reason){ ; }
        };
        
        //! \brief Graph component error - eg. invalid handle
        class invalid_graph_component_error: public graph_error {
        public:
            typedef enum {EDGE, NODE} component_type;
            
            explicit invalid_graph_component_error(const std::string & reason, component_type type) throw ()
                :graph_error(reason), m_type(type){ ; }
            
            //! \brief Whether this failed on node or edge
            component_type get_type() const throw () { return m_type; }

        private:
            component_type m_type;
        };

        //! \brief Graph of wrong topology given, expected other kind of topology
        class invalid_graph_topology: public graph_error {
        public:
            explicit invalid_graph_topology(const std::string & reason) throw ()
                :graph_error(reason){ ; }
        };
    }
}

#endif // KERAT_EXCEPTIONS_HPP
