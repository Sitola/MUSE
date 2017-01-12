/**
 * \file      viewport_projector.hpp
 * \brief     Adaptor that strips the bundles of contacts & information that do belong in given viewport
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-03-24 13:55 UTC+1
 * \copyright BSD
 */


#ifndef DTUIO_ADAPTOR_VIEWPORT_HPP
#define DTUIO_ADAPTOR_VIEWPORT_HPP

#include <kerat/kerat.hpp>
#include <dtuio/viewport.hpp>
#include <list>

namespace dtuio {
    namespace adaptors {
        class viewport_projector: public libkerat::adaptor {
        public:
            typedef std::list<sensor::viewport> viewport_list;
            
            viewport_projector(helpers::uuid uuid_to_follow, bool strip = true);
            viewport_projector(const sensor::viewport & viewport_to_match, bool strip = true);

            virtual ~viewport_projector();

            void notify(const libkerat::client * notifier);

            void purge();

            libkerat::bundle_stack get_stack() const;

            int process_bundle(const libkerat::bundle_handle & to_process, libkerat::bundle_handle & output_frame);

            static sensor::viewport calculate_bounding_viewport(const viewport_list & viewports);
        private:
            void process_viewport_updates(const libkerat::bundle_handle & to_process);
            
            static void get_corners(const sensor::viewport & vpt, libkerat::helpers::point_3d * corners);
            static libkerat::helpers::point_3d apply_viewport_cs(const libkerat::helpers::point_3d & original, const sensor::viewport & vpt);
            static bool in_viewport_box(const libkerat::helpers::point_3d & original, const sensor::viewport & vpt);
            
        protected:
            
            bool m_adaptive;
            helpers::uuid m_follow;
            sensor::viewport m_match;
            bool m_strip;

            libkerat::bundle_stack m_processed_frames;
        };
    }
}

#endif // DTUIO_ADAPTOR_VIEWPORT_HPP
