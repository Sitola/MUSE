/**
 * \file      viewport_scaler.hpp
 * \brief     Adaptor that scales the received viewport data
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-05-08 23:30 UTC+2
 * \copyright BSD
 */

#ifndef DTUIO_ADAPTOR_SCALER_HPP
#define DTUIO_ADAPTOR_SCALER_HPP

#include <kerat/kerat.hpp>
#include <dtuio/viewport.hpp>
#include <list>

namespace dtuio {
    namespace adaptors {
        class viewport_scaler: public libkerat::adaptor, public libkerat::server_adaptor {
        public:
            typedef std::list<sensor::viewport> viewport_list;
            
            viewport_scaler(const sensor::viewport & target_viewport);

            virtual ~viewport_scaler();

            void notify(const libkerat::client * notifier);

            void purge();
            
            const sensor::viewport & get_target_viewport() const { return m_target; }
            void set_target_viewport(const sensor::viewport & target);

            libkerat::bundle_stack get_stack() const;

            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);
            int process_bundle(libkerat::bundle_handle & to_process);

        protected:
            void apply_scale(libkerat::kerat_message * msg, const double & factor_x, const double & factor_y, const double & factor_z, const libkerat::helpers::point_3d & scale_center);
            
            sensor::viewport m_target;

            libkerat::bundle_stack m_processed_frames;
        };
    }
}

#endif // DTUIO_ADAPTOR_SCALER_HPP
