/**
 * \file      stdout_listener.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-22 10:49 UTC+1
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/stdout_listener.hpp>
#include <kerat/client.hpp>
#include <kerat/tuio_messages.hpp>
#include <kerat/bundle.hpp>
#include <iostream>

namespace libkerat {

    namespace listeners {

        stdout_listener::stdout_listener():m_output(std::cout){ ; }
        stdout_listener::stdout_listener(std::ostream& output_stream):m_output(output_stream){ ; }
        
        stdout_listener::~stdout_listener(){ ; }

        void stdout_listener::notify(const libkerat::client * notifier){

            libkerat::bundle_stack stack = notifier->get_stack();

            while (stack.get_length() > 0){
                libkerat::bundle_handle f = stack.get_update();
                process_bundle(f);
            }
        }
        
        void stdout_listener::process_bundle(libkerat::bundle_handle f){
            typedef libkerat::bundle_handle::const_iterator iterator;

            for (iterator i = f.begin(); i != f.end(); i++){
                print(*i);
                m_output << std::endl;
            }

            if (!f.empty()){ m_output << std::endl; }
        }

        bool stdout_listener::print(const libkerat::kerat_message* message){
            message->print(m_output);
            return true;
        }
        
    } // ns listeners

} // ns libkerat
