/**
 * \file      tuio_message_bounds.hpp
 * \brief     TUIO 2.0 bounds (/tuio2/bnd, /tuio2/b3d)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-29 12:41 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_BOUNDS_HPP
#define KERAT_TUIO_MESSAGE_BOUNDS_HPP

#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/message_helpers.hpp>
#include <ostream>

namespace libkerat {
    namespace message {

        /**
         * \brief TUIO 2.0 bounds (/tuio2/bnd, /tuio2/b3d)
         * 
         * \note For TUIO 2.0 draft (non)compliance, see below
         * \see \ref normalized-coordinates \ref normalized-distance
         * \ref normalized-velocity \ref normalized-acceleration \ref normalized-pressure
         * \ref normalized-rotation
         */
        class bounds: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::point_3d,
            public libkerat::helpers::angle_3d,
            public libkerat::helpers::velocity_3d,
            public libkerat::helpers::rotation_velocity_3d,
            public libkerat::helpers::movement_acceleration,
            public libkerat::helpers::rotation_acceleration,
            public libkerat::helpers::message_output_mode,
            public libkerat::helpers::movable_3d,
            public libkerat::helpers::scalable_independent_3d,
            public libkerat::helpers::rotatable_cs_3d,
            public libkerat::helpers::rotatable_independent_3d
        
        {
        public:

            /**
             *  \brief Creates new, empty TUIO bounds
             * 
             * All message components (and corresponding helpers) are initialized
             * through their default constructors. The message output mode is set
             * to \ref OUTPUT_MODE_BOTH.
             */
            bounds();

            /**
             * \brief Creates new, short 2D TUIO bounds
             * 
             * All other message components (and corresponding helpers) are 
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_2D.
             * 
             * \param session_id - session id of the contact
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param angle      - the angle between the bounding box's inner elipse main axis and X axis in XY plane
             * \param bbox_width   - width of the region bounding box
             * \param bbox_height  - height of the region bounding box
             * \param region_area  - area of the original region
             */
            bounds(const session_id_t session_id,
                const coord_t x, const coord_t y,
                const angle_t angle,
                const distance_t bbox_width, const distance_t bbox_height,
                const area_t region_area
            );

            /**
             * \brief Creates new, fully specified 2D TUIO bounds
             * 
             * All other message components (and corresponding helpers) are 
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_2D.
             * 
             * \param session_id - session id of the contact
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param angle      - the angle between the bounding box's inner elipse main axis and X axis in XY plane
             * \param bbox_width   - width of the region bounding box
             * \param bbox_height  - height of the region bounding box
             * \param region_area  - area of the original region
             * \param x_velocity   - X axis component of the movement velocity
             * \param y_velocity   - Y axis component of the movement velocity
             * \param rotation_velocity - the rotation velocity
             * \param movement_accel - overall acceleration in the movement direction
             * \param rotation_accel - overall rotation acceleration in the rotation direction
             */
            bounds(const session_id_t session_id,
                const coord_t x, const coord_t y,
                const angle_t angle,
                const distance_t bbox_width, const distance_t bbox_height,
                const area_t region_area,
                const velocity_t x_velocity, const velocity_t y_velocity,
                const rotation_velocity_t rotation_velocity,
                const accel_t movement_accel, const rotation_accel_t rotation_accel
            );

            /**
             * \brief Creates new, short 3D TUIO bounds
             * 
             * All other message components (and corresponding helpers) are 
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_3D.
             * 
             * \param session_id - session id of the contact
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param z          - Z coordinate
             * \param yaw_angle    - the yaw angle of the bounding box's main axis
             * \param pitch_angle  - the pitch angle of the bounding box's main axis
             * \param roll_angle   - the roll angle of the bounding box's main axis
             * \param bbox_width - width of the bounding box
             * \param bbox_height - height of the bounding box
             * \param bbox_depth - depth of the bounding box
             * \param region_volume - original region volume
             */
            bounds(const session_id_t session_id,
                const coord_t x, const coord_t y, const coord_t z,
                const angle_t yaw_angle, const angle_t pitch_angle, const angle_t roll_angle,
                const distance_t bbox_width, const distance_t bbox_height, const distance_t bbox_depth,
                const volume_t region_volume
            );

            /**
             * \brief Creates new, fully specified 3D TUIO bounds
             * 
             * All other message components (and corresponding helpers) are 
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_3D.
             * 
             * \param session_id - session id of the contact
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param z          - Z coordinate
             * \param yaw_angle    - the yaw angle of the bounding box's main axis
             * \param pitch_angle  - the pitch angle of the bounding box's main axis
             * \param roll_angle   - the roll angle of the bounding box's main axis
             * \param bbox_width - width of the bounding box
             * \param bbox_height - height of the bounding box
             * \param bbox_depth - depth of the bounding box
             * \param region_volume - original region volume
             * \param x_velocity - X axis component of the movement velocity
             * \param y_velocity - Y axis component of the movement velocity
             * \param z_velocity - Z axis component of the movement velocity
             * \param yaw_velocity  - the yaw rotation velocity
             * \param pitch_velocity - the pitch rotation velocity
             * \param roll_velocity - the roll rotation velocity
             * \param movement_accel - overall acceleration in the movement direction
             * \param rotation_accel - overall rotation acceleration in the rotation direction
             */
            bounds(const session_id_t session_id,
                const coord_t x, const coord_t y, const coord_t z,
                const angle_t yaw_angle, const angle_t pitch_angle, const angle_t roll_angle,
                const distance_t bbox_width, const distance_t bbox_height, const distance_t bbox_depth,
                const volume_t region_volume,
                const velocity_t x_velocity, const velocity_t y_velocity, const velocity_t z_velocity,
                const rotation_velocity_t yaw_velocity, const rotation_velocity_t pitch_velocity, const rotation_velocity_t roll_velocity,
                const accel_t movement_accel, const rotation_accel_t rotation_accel
            );

            bounds & operator=(const bounds & second);
            bool operator == (const bounds & second) const ;
            inline bool operator != (const bounds & second) const { return !operator==(second); }

            /**
             * \brief Get the bounding box width
             * \return width or 0 if default
             */
            inline distance_t get_width() const { return m_width; }

            /**
             * \brief Set the bounding box width
             * \param bbox_width - width of the bounding box
             * \return previous setting
             */
            distance_t set_width(distance_t bbox_width);

            /**
             * \brief Get the bounding box height
             * \return height or 0 if default
             */
            inline distance_t get_height() const { return m_height; }

            /**
             * \brief Set the bounding box height
             * \param bbox_height - height of the bounding box
             * \return previous setting
             */
            distance_t set_height(distance_t bbox_height);


            /**
             * \brief Get the region area
             * \return area or 0 if default
             */
            inline area_t get_area() const { return m_area; }

            /**
             * \brief Set the region area
             * \param region_area - area 
             * \return previous setting
             */
            area_t set_area(area_t region_area);

            /**
             * \brief Get the bounding box depth
             * \return depth or 0 if default
             */
            inline distance_t get_depth() const { return m_depth; }

            /**
             * \brief Set the bounding box depth
             * \param bbox_depth - depth of the bounding box
             * \return previous setting
             */
            distance_t set_depth(distance_t bbox_depth);

            /**
             * \brief Get the region volume
             * \return volume or 0 if default
             */
            inline volume_t get_volume() const { return m_vol; }

            /**
             * \brief Set the region volume
             * \param region_volume - volume
             * \return previous setting
             */
            volume_t set_volume(volume_t region_volume);
            
            /**
             * \brief Checks for extended attributes
             * 
             * Checks whether at least one of the following attributes are set
             * depending on the message output mode
             * 
             * \li velocity
             * \li rotation velocity
             * \li acceleration
             * \li rotation acceleration
             * 
             * \return true if any of the above values are set for given message output mode, false otherwise
             */
            bool is_extended() const;

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;
            
            // movable, scalable and rotatable helpers override
            void scale_x(float factor);
            void scale_y(float factor);
            void scale_z(float factor);
            void move_x(coord_t factor);
            void move_y(coord_t factor);
            void move_z(coord_t factor);
            void rotate_by(angle_t angle);
            void rotate_pitch(angle_t angle);
            void rotate_roll(angle_t angle);
            void rotate_by(angle_t angle, const helpers::point_2d & center);
            void rotate_pitch(angle_t angle, const helpers::point_3d & center);
            void rotate_roll(angle_t angle, const helpers::point_3d & center);

            //! \brief OSC path for the 2D bounds
            static const char * PATH_2D;
            //! \brief OSC path for the 3D bounds
            static const char * PATH_3D;

        private:

            bool imprint_lo_messages(lo_bundle target) const;

            // bbox width
            distance_t m_width;
            // bbox height
            distance_t m_height;
            // bbox area
            area_t m_area;
            // bbox depth
            distance_t m_depth;
            // bbox volume
            volume_t m_vol;

        }; // cls bounds

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::bounds &  msg_bnd);

#endif // KERAT_TUIO_MESSAGE_BOUNDS_HPP
