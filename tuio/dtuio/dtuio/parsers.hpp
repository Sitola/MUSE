/**
 * \file      parsers.hpp
 * \brief     provides the headers for all dtuio parsers
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-28 19:33 UTC+1
 * \copyright BSD
 */

#ifndef DTUIO_PARSERS_HPP
#define DTUIO_PARSERS_HPP

#include <lo/lo.h>
#include <list>
#include <kerat/parsers.hpp>

namespace dtuio {
    //! \brief Internal dTUIO algorithms & functions.
    namespace internals {
        //! \brief Here all the dTUIO message parsers are located
        namespace parsers {

            //! \brief parser for \ref dtuio::sensor_topology::neighbour
            //! \note See libkerat::simple_client documentation for details
            bool parse_topology_nbr(libkerat::internals::convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)) );

            //! \brief parser for \ref dtuio::sensor_topology::group_member
            //! \note See libkerat::simple_client documentation for details
            bool parse_topology_grm(libkerat::internals::convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)) );

            //! \brief parser for \ref dtuio::sensor::sensor_properties
            //! \note See libkerat::simple_client documentation for details
            bool parse_sensor(libkerat::internals::convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)) );

            //! \brief parser for \ref dtuio::gesture::gesture_identification
            //! \note See libkerat::simple_client documentation for details
            bool parse_gesture_identification(libkerat::internals::convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)) );

            //! \brief parser for \ref dtuio::sensor::viewport
            //! \note See libkerat::simple_client documentation for details
            bool parse_viewport(libkerat::internals::convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)) );
            
        } // ns parsers
        
        /**
         * \brief Gets list of all dTUIO message convertors
         * This function allows for easy dtuio adaptation onto 3'rd party
         * TUIO 2.0 client implementations as well as native client implementations
         * \return List of convertors corresponding to all dTUIO message parsers
         */
        libkerat::internals::convertor_list get_dtuio_convertors();
        
    } // ns internals
} // ns dtuio

#endif // DTUIO_PARSERS_HPP
