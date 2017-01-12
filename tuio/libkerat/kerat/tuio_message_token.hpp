/**
 * \file      tuio_message_token.hpp
 * \brief     TUIO 2.0 token (/tuio2/tok, /tuio2/t3d)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-25 15:51 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_TOKEN_HPP
#define KERAT_TUIO_MESSAGE_TOKEN_HPP

#include <ostream>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/message_helpers.hpp>

namespace libkerat {
    namespace message {

        /**
         * \brief TUIO 2.0 token (/tuio2/tok, /tuio2/t3d)
         * 
         * \note For TUIO 2.0 draft (non)compliance, see below
         * \see \ref normalized-coordinates
         * \ref normalized-velocity \ref normalized-acceleration
         * \ref normalized-rotation
         */
        class token: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::contact_type_user,
            public libkerat::helpers::contact_component,
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
             * \brief Creates new, generic empty TUIO token
             * 
             * All message components (and corresponding helpers) are initialized
             * through their default constructors. The message output mode is set
             * to \ref OUTPUT_MODE_BOTH.
             */
            token();

            /**
             * \brief Creates new, short 2D TUIO token
             * 
             * All other message components (and corresponding helpers) are 
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_2D .
             * 
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param angle      - the angle between the object's main axis and X axis in XY plane
             */
            token(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y,
                const angle_t angle
            );

            /**
             * \brief Creates new, fully specified 2D TUIO token
             * 
             * All other message components (and corresponding helpers) are
             * initialized through their default constructors. The message output
             * mode is set to \ref OUTPUT_MODE_2D.
             * 
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param angle      - the angle between the object's main axis and X axis in XY plane
             * \param x_velocity - X axis component of the movement velocity
             * \param y_velocity - Y axis component of the movement velocity
             * \param rotation_velocity - the rotation velocity
             * \param movement_accel    - overall acceleration in the movement direction
             * \param rotation_accel    - overall rotation acceleration in the rotation direction
             */
            token(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y,
                const angle_t angle,
                const velocity_t x_velocity, const velocity_t y_velocity,
                const rotation_velocity_t rotation_velocity,
                const accel_t movement_accel, const rotation_accel_t rotation_accel
            );

            /**
             * \brief Creates new, short 3D TUIO token
             * 
             * All other message components (and corresponding helpers) are initialized 
             * through their default constructors. The message output mode is
             * set to \ref OUTPUT_MODE_3D.
             * 
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param z          - Z coordinate
             * \param yaw        - the angle between the object's main axis and X axis
             * \param pitch      - the angle between the object's main axis and Y axis
             * \param roll       - the angle between the object's main axis and Z axis
             */
            token(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y, const coord_t z,
                const angle_t yaw, const angle_t pitch, const angle_t roll
            );

            /**
             * \brief Creates new, fully specified 3D TUIO token
             * 
             * The message output mode is set to \ref OUTPUT_MODE_3D.
             * 
             * \param session_id - session id of the contact
             * \param type_id    - type id of the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param user_id    - id of the user manipulating the contact (see corresponding constants of the \ref helpers::contact_type_user helper)
             * \param component_id - component id of the contact - each contact can have several components
             * \param x          - X coordinate
             * \param y          - Y coordinate
             * \param z          - Z coordinate
             * \param yaw        - the angle between the object's main axis and X axis
             * \param pitch      - the angle between the object's main axis and Y axis
             * \param roll       - the angle between the object's main axis and Z axis
             * \param x_velocity - X axis component of the movement velocity
             * \param y_velocity - Y axis component of the movement velocity
             * \param z_velocity - Z axis component of the movement velocity
             * \param yaw_velocity   - the yaw rotation velocity
             * \param pitch_velocity - the pitch rotation velocity
             * \param roll_velocity  - the roll rotation velocity
             * \param movement_accel - overall acceleration in the movement direction
             * \param rotation_accel - overall rotation acceleration in the rotation direction
             */
            token(const session_id_t session_id, const type_id_t type_id, const user_id_t user_id, const component_id_t component_id,
                const coord_t x, const coord_t y, const coord_t z,
                const angle_t yaw, const angle_t pitch, const angle_t roll,
                const velocity_t x_velocity, const velocity_t y_velocity, const velocity_t z_velocity,
                const rotation_velocity_t yaw_velocity, const rotation_velocity_t pitch_velocity, const rotation_velocity_t roll_velocity,
                const accel_t movement_accel, const rotation_accel_t rotation_accel
            );

            token(const token & original);

            
            token & operator=(const token & second);
            bool operator == (const token & second) const ;
            inline bool operator != (const token & second) const { return !operator==(second); }

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
            bool is_extended() const ;

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            // overrides for movable and rotatable
            void move_x(coord_t factor);
            void move_y(coord_t factor);
            void move_z(coord_t factor);
            void scale_x(float factor);
            void scale_y(float factor);
            void scale_z(float factor);
            void rotate_by(angle_t angle, const helpers::point_2d & center);
            void rotate_pitch(angle_t angle, const helpers::point_3d & center);
            void rotate_roll(angle_t angle, const helpers::point_3d & center);
            void rotate_by(angle_t angle);
            void rotate_pitch(angle_t angle);
            void rotate_roll(angle_t angle);
            
            //! \brief OSC path for the 2D token
            static const char * PATH_2D;
            //! \brief OSC path for the 3D token
            static const char * PATH_3D;

        private:

            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls token
        
    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::token &  msg_tok);

#endif // KERAT_TUIO_MESSAGE_TOKEN_HPP
