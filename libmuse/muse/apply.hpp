/**
 * \file      apply.hpp
 * \brief     Provides the means of matching the pointer inside the bounding box generated by the client
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-18 16:55 UTC+1
 * \copyright BSD
 */

#ifndef MUSE_APPLY_HPP
#define MUSE_APPLY_HPP

#include <kerat/kerat.hpp>
#include <sys/types.h>
#include <regex.h>
#include <map>

namespace muse {
    namespace aggregators {

        class apply: public libkerat::adaptor {
        public:
            typedef std::vector<libkerat::adaptor *> adaptor_vector;
            

            apply(const std::string & matching_regex, const adaptor_vector & adaptors_to_apply, bool free_adaptors = false);
            ~apply();
            
            void set_adaptors(const adaptor_vector & adaptors_to_apply);

            void notify(const libkerat::client * notifier);

            libkerat::bundle_stack get_stack() const { return m_processed_frames; }

            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);

            void purge();

            virtual bool load(int count = 1);
            virtual bool load(int count, struct timespec timeout);

        private:
            bool is_bundle_matching(const libkerat::bundle_handle & bundle);

            regex_t m_regex;
            bool m_free_adaptors;
            adaptor_vector m_adaptors;

            libkerat::bundle_stack m_processed_frames;

        };
    }
}


#endif // MUSE_APPLY_HPP
