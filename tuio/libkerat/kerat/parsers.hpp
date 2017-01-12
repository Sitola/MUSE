/**
 * \file      parsers.hpp
 * \brief     provides the headers for all standard parsers
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-16 14:53 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_PARSERS_HPP
#define KERAT_PARSERS_HPP

#include <kerat/message.hpp>
#include <kerat/utils.hpp>
#include <lo/lo.h>
#include <vector>

namespace libkerat {
    namespace internals {
        typedef std::vector<libkerat::kerat_message *> convertor_output_container;

        /**
         * \brief Function pointer typedef to the message converting function
         * \param[out] results - list to store the generated messages to
         * \param path - the received message path
         * \param types - the received message typetag
         * \param argv - array of the received message arguments
         * \param argc - argument count of the original osc message
         * \param user_data - the user data to give to the convertor
         */
        typedef bool (*message_convertor)(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data);

        //! Structure holding OSC message path to message convertor binding
        struct message_convertor_entry: public internals::callback_setting<message_convertor> {
        private:
            typedef callback_setting<message_convertor> callback_stg;
        
        public:
            //! \brief Create a new, empty convertor entry
            message_convertor_entry();

            /**
             * \brief Create a new convertor entry
             * \param path - OSC message path for which this convertor should be applied
             * \param conv - convertor function
             * \param data - user data to run the convertor function with
             */
            message_convertor_entry(std::string path, message_convertor conv, void * data = NULL);

            /**
             * \brief Create a new convertor entry
             * \param path - list of OSC message paths for which this convertor should be applied
             * \param conv - convertor function
             * \param data - user data to run the convertor function with
             */
            message_convertor_entry(std::list<std::string> paths, message_convertor conv, void * data = NULL);

            bool operator==(const message_convertor_entry & other);

            //! \brief OSC Message paths that this convertor entry responds to
            std::list<std::string> m_osc_paths;

        };
        
        typedef std::list<message_convertor_entry> convertor_list;
        convertor_list get_libkerat_convertors();
        
        namespace parsers {

            bool parse_alv(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)) );
            bool parse_frm(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)) );

            bool parse_ptr_2d(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_tok_2d(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_bnd_2d(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_sym(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc __attribute__((unused)), void * user_data __attribute__((unused)));

            bool parse_ptr_3d(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_bnd_3d(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_tok_3d(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused))); 

            bool parse_chg(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_ocg(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_icg(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_skg_2d(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_skg_3d(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_svg(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_arg(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_raw(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc __attribute__((unused)), void * user_raw __attribute__((unused)));

            bool parse_ctl(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_dat(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc __attribute__((unused)), void * user_data __attribute__((unused)));
            bool parse_sig(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));

            bool parse_ala(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_coa(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_lia(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_lla(convertor_output_container & results, const char * path, const char * types, lo_arg ** argv, int argc, void * user_data __attribute__((unused)));
            bool parse_lta(convertor_output_container & results  __attribute__((unused)), const char* path __attribute__((unused)), const char* types __attribute__((unused)), lo_arg** argv __attribute__((unused)), int argc __attribute__((unused)), void* user_data __attribute__((unused)));
            
            bool parse_generic_osc_message(libkerat::kerat_message ** result, const char * path, const lo_message & message);

        }
    }
}

#endif // KERAT_PARSERS_HPP
