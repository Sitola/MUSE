/**
 * \file      viewport.hpp
 * \brief     Define the message that describes the viewport
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-03-24 12:56 UTC+1
 * \copyright BSD
 */


#ifndef DTUIO_VIEWPORT_HPP
#define DTUIO_VIEWPORT_HPP

#include <kerat/kerat.hpp>
#include <dtuio/typedefs.hpp>
#include <dtuio/helpers.hpp>
#include <string>
#include <stdexcept>

namespace dtuio {
    namespace sensor {
        //! \brief Describe sensor viewport (that is sensor's position and box)
        class viewport: 
            public libkerat::kerat_message, 
            public dtuio::helpers::uuid, 
            public libkerat::helpers::point_3d,
            public libkerat::helpers::angle_3d,
            public libkerat::helpers::movable_3d,
            public libkerat::helpers::rotatable_cs_3d
        {
        public:
            viewport();
            viewport(const helpers::uuid & viewport_uuid, const libkerat::helpers::point_3d & center, const libkerat::helpers::angle_3d & angle, const libkerat::dimmension_t width, const libkerat::dimmension_t height, const libkerat::dimmension_t depth = 0);
            viewport(const helpers::uuid & viewport_uuid, const libkerat::helpers::point_3d & center, const libkerat::dimmension_t width, const libkerat::dimmension_t height, const libkerat::dimmension_t depth = 0);
            viewport(const helpers::uuid & viewport_uuid, const libkerat::dimmension_t width, const libkerat::dimmension_t height, const libkerat::dimmension_t depth = 0);
            viewport(const viewport & original);
            virtual ~viewport();
            
            viewport & operator=(const viewport & second);
            bool operator == (const viewport & second) const ;
            inline bool operator != (const viewport & second) const { return !operator==(second); }
            
            // movable, scalable and rotatable helpers override
            void scale_x(float factor);
            void scale_y(float factor);
            void scale_z(float factor);
            void move_x(libkerat::coord_t factor);
            void move_y(libkerat::coord_t factor);
            void move_z(libkerat::coord_t factor);
            void rotate_by(libkerat::angle_t angle);
            void rotate_pitch(libkerat::angle_t angle);
            void rotate_roll(libkerat::angle_t angle);
            void rotate_by(libkerat::angle_t angle, const libkerat::helpers::point_2d & center);
            void rotate_pitch(libkerat::angle_t angle, const libkerat::helpers::point_3d & center);
            void rotate_roll(libkerat::angle_t angle, const libkerat::helpers::point_3d & center);
            
            libkerat::kerat_message * clone() const ;
            void print(std::ostream & output) const ;
            
            void set_width(libkerat::distance_t viewport_width);
            inline libkerat::distance_t get_width() const { return m_width; }

            void set_height(libkerat::distance_t viewport_height);
            inline libkerat::distance_t get_height() const { return m_height; }
            
            void set_depth(libkerat::distance_t viewport_depth);
            inline libkerat::distance_t get_depth() const { return m_depth; }

            static const char * PATH;
            
        private:
            bool imprint_lo_messages(lo_bundle target) const;
            
        protected:
            void init(const helpers::uuid & viewport_uuid, const libkerat::helpers::point_3d & center, const libkerat::helpers::angle_3d & angle, const libkerat::dimmension_t width, const libkerat::dimmension_t height, const libkerat::dimmension_t depth);
            
            libkerat::dimmension_t m_width;
            libkerat::dimmension_t m_height;
            libkerat::dimmension_t m_depth;
        
        };
    }
}

std::ostream & operator<<(std::ostream & output, const dtuio::sensor::viewport & box);

#endif // DTUIO_VIEWPORT_HPP
