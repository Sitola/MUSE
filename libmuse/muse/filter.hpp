/**
 * \file      filter.hpp
 * \brief     Provides the means of matching the pointer inside the bounding box generated by the client
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-18 16:55 UTC+1
 * \copyright BSD
 */

#ifndef MUSE_FILTER_HPP
#define MUSE_FILTER_HPP

#include <kerat/kerat.hpp>
#include <sys/types.h>
#include <regex.h>
#include <map>

namespace muse {
    namespace aggregators {

        class filter: public libkerat::adaptor {
        public:

            filter(const std::string & matching_regex = ".*", libkerat::listener * sink = NULL);
            ~filter();

            virtual void del_listener(libkerat::listener * lstnr);
            virtual void add_listener(libkerat::listener * lstnr);
            virtual void set_sink(libkerat::listener * sink);
            
            void notify(const libkerat::client * notifier);

            virtual libkerat::bundle_stack get_stack() const ;

            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);

            virtual void purge();

            virtual bool load(int count = 1);
            virtual bool load(int count, struct timespec timeout);

        private:

            bool is_bundle_matching(const libkerat::bundle_handle & bundle);

            regex_t m_regex;
            libkerat::listener * m_sink;
            bool m_tell_sink;

            libkerat::bundle_stack m_filtered_frames;
            libkerat::bundle_stack m_sinked_frames;

        };
    }
}


#endif // MUSE_FILTER_HPP
