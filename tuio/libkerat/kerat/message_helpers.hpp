/**
 * \file      message_helpers.hpp
 * \brief     Provides the message helpers for libkerat TUIO 2.0 messages
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-02 23:12 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_MESSAGE_HELPERS_HPP
#define KERAT_MESSAGE_HELPERS_HPP

#include <kerat/typedefs.hpp>
//#include <kerat/utils.hpp>
#include <kerat/graph.hpp>
#include <cmath>

namespace libkerat {

    namespace helpers {

        // forward, libkerat helpers
        
        class point_2d;
        class point_3d;
        class velocity_2d;
        class velocity_3d;
        class angle_2d;
        class angle_3d;
        class rotation_velocity_2d;
        class rotation_velocity_3d;
        class movement_acceleration;
        class rotation_acceleration;
        class contact_session;
        class contact_type_user;
        class contact_component;
        class message_output_mode;
        
        class scalable_independent_2d;
        class scalable_independent_3d;
        class scalable_2d;
        class scalable_3d;
        class rotatable_independent_2d;
        class rotatable_independent_3d;
        class rotatable_cs_2d;
        class rotatable_cs_3d;
        class movable_2d;
        class movable_3d;

        //! \brief 2D point with the standard algebraic operations plus lexicographical order due to axis ordering
        class point_2d {
        public:
            /**
             * \brief Creates a new point on given coordinates
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-coordinates
             */
            point_2d(const coord_t x, const coord_t y);

            //! \brief Creates a new point on coordinates [0, 0]
            point_2d();

            point_2d & operator=(const point_2d & second);
            bool operator== (const point_2d & second) const ;
            inline bool operator!=(const point_2d & second) const { return !operator==(second); }

            point_2d operator+(const point_2d & second) const ;
            point_2d operator-(const point_2d & second) const ;
            point_2d operator/(const coord_t factor) const ;
            point_2d operator*(const coord_t factor) const ;

            point_2d & operator+=(const point_2d & second);
            point_2d & operator-=(const point_2d & second);
            point_2d & operator/=(const coord_t factor);
            point_2d & operator*=(const coord_t factor);
            
            /**
             * \brief Checks the lexicographical order against given point
             * \param second - point to compare to
             * \return true if belongs before, false othervise
             */
            bool operator<(const point_2d & second) const ;

            /**
             * \brief Gets the X axis ccoordinate
             * \return x coordinate
             */
            inline coord_t get_x() const { return m_x_pos; }

            /**
             * \brief Sets the X axis coordinate
             * \param x - x coordinate to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-coordinates
             * \return previous setting
             */
            coord_t set_x(const coord_t x);

            /**
             * \brief Gets the Y axis coordinate
             * \return y coordinate
             */
            inline coord_t get_y() const { return m_y_pos; }

            /**
             * \brief Sets the Y axis coordinate
             * \param y - y coordinate to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-coordinates
             * \return previous setting
             */
            coord_t set_y(const coord_t y);

        protected:

            coord_t m_x_pos;
            coord_t m_y_pos;

        }; // cls point_2d

        //! \brief 3D point with the standard algebraic operations plus lexicographical order due to axis ordering
        class point_3d: public point_2d {
        public:

            //! \brief Creates a new point on given coordinates
            point_3d(const coord_t x, const coord_t y, const coord_t z);
            point_3d(const point_2d & pt);

            //! \brief Creates a new point on coordinates [ 0, 0, 0]
            point_3d();

            point_3d & operator=(const point_3d & second);
            bool operator== (const point_3d & second) const ;
            inline bool operator!=(const point_3d & second) const { return !operator==(second); }

            point_3d operator+(const point_3d & second) const ;
            point_3d operator-(const point_3d & second) const ;
            point_3d operator/(const coord_t factor) const ;
            point_3d operator*(const coord_t factor) const ;

            point_3d & operator+=(const point_3d & second);
            point_3d & operator-=(const point_3d & second);
            point_3d & operator/=(const coord_t factor);
            point_3d & operator*=(const coord_t factor);

            /**
             * \brief Checks the lexicographical order against given point
             * \param second - point to compare to
             * \return true if belongs before, false othervise
             */
            bool operator<(const point_3d & second) const ;
            
            /**
             * \brief Gets the Z axis ccoordinate
             * \return z coordinate
             */
            inline coord_t get_z() const { return m_z_pos; }

            /**
             * \brief Sets the Z axis coordinate
             * \param z - z coordinate to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-coordinates
             * \return previous setting
             */
            coord_t set_z(const coord_t z);

        protected:

            coord_t m_z_pos;
        }; // cls point_3d

        //! \brief 2D velocity with individual components
        class velocity_2d {
        public:

            //! \brief Creates a new velocity message helper with nil elementar velocies
            velocity_2d();

            /**
             * \brief Creates a new velocity message helper with given elementar velocies
             * \param x_velocity - velocity in the x axis dirrection
             * \param y_velocity - velocity in the y axis dirrection
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-velocity
             * where 1 reads as movement along the whole axis per second
             */
            velocity_2d(const velocity_t x_velocity, const velocity_t y_velocity);

            velocity_2d & operator=(const velocity_2d & second);
            bool operator== (const velocity_2d & second) const ;
            inline bool operator!=(const velocity_2d & second) const { return !operator==(second); }

            /**
             * \brief Get the movement velocity in X axis direction
             * \return velocity or 0 if unset (see \ref has_velocity )
             */
            inline velocity_t get_x_velocity() const { return m_x_vel; }

            /**
             * \brief Set the movement velocity in X axis direction
             * \param x_velocity - velocity in direction of X axis
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-velocity
             * \return previous setting
             */
            velocity_t set_x_velocity(velocity_t x_velocity);

            /**
             * \brief Get the movement velocity in Y axis direction
             * \return velocity or 0 if unset (see \ref has_velocity )
             */
            inline velocity_t get_y_velocity() const { return m_y_vel; }

            /**
             * \brief Set the movement velocity in Y axis direction
             * \param y_velocity - velocity in direction of Y axis
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-velocity
             * \return previous setting
             */
            velocity_t set_y_velocity(velocity_t y_velocity);
            
            /**
             * \brief Checks whether the velocity any direction is set
             * \return if any velocity is non-zero
             */
            bool has_velocity() const ;
            
            /**
             * \brief Get the overall velocity
             * The overall velocity is calculated as euclidan norm of the velocity vector
             * \return overall velocity
             */
            velocity_t get_overall_velocity() const ;

        protected:

            velocity_t m_x_vel;
            velocity_t m_y_vel;

        }; // cls velocity_2d

        //! \brief 3D velocity with individual components
        class velocity_3d: public velocity_2d {
        public:

            //! \brief Creates a new velocity message helper with nil elementar velocies
            velocity_3d();

            /**
             * \brief Creates a new velocity message helper with given elementar velocies
             * \param x_velocity - velocity in the x axis dirrection
             * \param y_velocity - velocity in the y axis dirrection
             * \param z_velocity - velocity in the z axis dirrection
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-velocity
             */
            velocity_3d(const velocity_t x_velocity, const velocity_t y_velocity, const velocity_t z_velocity);

            velocity_3d & operator=(const velocity_3d & second);
            bool operator== (const velocity_3d & second) const ;
            inline bool operator!=(const velocity_3d & second) const { return !operator==(second); }

            /**
             * \brief Get the movement velocity in Z axis direction
             * \return velocity or 0 if unset (see \ref has_velocity )
             */
            inline velocity_t get_z_velocity() const { return m_z_vel; }

            /**
             * \brief Set the movement velocity in Z axis direction
             * \param z_velocity - velocity in direction of Z axis
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-velocity
             * \return previous setting
             */
            velocity_t set_z_velocity(velocity_t z_velocity);

            /**
             * \brief Checks whether the velocity any direction is set
             * \return if any velocity is non-zero
             */
            bool has_velocity() const ;
            
            /**
             * \brief Get the overall velocity
             * The overall velocity is calculated as euclidan norm of the velocity vector
             * \return overall velocity
             */
            velocity_t get_overall_velocity() const ;
            
        protected:

            velocity_t m_z_vel;

        }; // cls velocity_3d

        //! \brief 2D angle
        class angle_2d {
        public:
            
            //! \brief Creates a new empty 2D angle message helper
            angle_2d();

            //! \brief Creates a new 2D angle message helper containing the given angle
            angle_2d(const angle_t angle);

            angle_2d & operator=(const angle_2d & second);
            bool operator== (const angle_2d & second) const ;
            inline bool operator!=(const angle_2d & second) const { return !operator==(second); }

            /**
             * \brief Get the angle
             * \returns the angle if set or 0 if unset
             */
            inline angle_t get_angle() const { return m_yaw; }

            /**
             * \brief Set the angle 
             * \param angle - angle to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-rotation
             * \see libkerat::normalize libkerat::strip_angle_period
             * \returns previous setting
             */
            angle_t set_angle(const angle_t angle);

        protected:

            angle_t m_yaw;

        }; // cls angle_2d

        /**
         * \brief 3D individual agles (yaw, pitch and roll)
         * \note Since 3D angle helper extends 2D angle helper,
         * the yaw angle maps to the 2D angle value.
         */
        class angle_3d: public angle_2d {
        public:

            //! \brief Creates a new empty 3D angle message helper
            angle_3d();

            //! \brief Creates a new 3D angle message helper containing the given angles
            angle_3d(const angle_t yaw, const angle_t pitch, const angle_t roll);

            angle_3d & operator=(const angle_3d & second);
            bool operator== (const angle_3d & second) const ;
            inline bool operator!=(const angle_3d & second) const { return !operator==(second); }

            /**
             * \brief Get the yaw angle
             * \note Alias for \ref angle_2d::get_angle
             * \returns The yaw angle if set or 0 if unset
             */
            inline angle_t get_yaw() const { return get_angle(); }

            /**
             * \brief Set the yaw angle 
             * \note Alias for \ref angle_2d::set_angle
             * \param yaw - yaw angle to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-rotation
             * \see libkerat::normalize libkerat::strip_angle_period
             * \returns Previous setting
             */
            inline angle_t set_yaw(const angle_t yaw){ return angle_2d::set_angle(yaw);  }

            /**
             * \brief Get the pitch angle
             * \returns The pitch angle if set or 0 if unset
             */
            inline angle_t get_pitch() const { return m_pitch; }

            /**
             * \brief Set the pitch angle 
             * \param pitch - pitch angle to be set, for TUIO 2.0 draft compliance
             * use the normalized &lt;0; 2*PI&gt; range
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-rotation
             * \see libkerat::normalize libkerat::strip_angle_period
             * \returns Previous setting
             */
            angle_t set_pitch(const angle_t pitch);

            /**
             * \brief Get the roll angle
             * \returns The roll angle if set or 0 if unset
             */
            inline angle_t get_roll() const { return m_roll; }

            /**
             * \brief Set the roll angle 
             * \param roll - roll angle to be set, for TUIO 2.0 draft compliance
             * use the normalized &lt;0; 2*PI&gt; range
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-rotation
             * \see libkerat::normalize libkerat::strip_angle_period
             * \returns Previous setting
             */
            angle_t set_roll(const angle_t roll);

        protected:

            angle_t m_pitch;
            angle_t m_roll;

        }; // cls angle_3d

        //! \brief 2D rotation velocity
        class rotation_velocity_2d {
        public:

            //! \brief Creates a new empty 2D rotation velocity message helper
            rotation_velocity_2d();

            //! \brief Creates a new 2D rotation velocity message helper
            rotation_velocity_2d(const rotation_velocity_t yaw_velocity);

            rotation_velocity_2d & operator=(const rotation_velocity_2d & second);
            bool operator== (const rotation_velocity_2d & second) const ;
            inline bool operator!=(const rotation_velocity_2d & second) const { return !operator==(second); }

            /**
             * \brief Get the rotation velocity
             * \return rotation velocity or 0 if not set
             */
            inline rotation_velocity_t get_rotation_velocity() const { return m_yaw_vel; }

            /**
             * \brief Set the rotation velocity
             * \param rotation_velocity - rotation velocity to be set
             * \note For TUIO 2.0 draft compliance use the normalized &lt;0; 1&gt; range,
             * where 1 reads as 1 full rotation per second
             * \return previous setting
             */
            rotation_velocity_t set_rotation_velocity(const rotation_velocity_t rotation_velocity);

        protected:

            rotation_velocity_t m_yaw_vel;

        }; // cls rotation_velocity_2d

        /**
         * \brief 3D rotation velocities (yaw, pitch and roll)
         * \note Since 3D rotation velocity helper extends 2D rotation velocity helper,
         * the yaw angle velocity maps to the 2D angle velocity value.
         */
        class rotation_velocity_3d: public rotation_velocity_2d {
        public:

            //! \brief Creates a new empty 3D rotation velocity message helper
            rotation_velocity_3d();

            //! \brief Creates a new 3D rotation velocity message helper
            rotation_velocity_3d(const rotation_velocity_t yaw_velocity, const rotation_velocity_t pitch_velocity, const rotation_velocity_t roll_velocity);

            rotation_velocity_3d & operator=(const rotation_velocity_3d & second);
            bool operator== (const rotation_velocity_3d & second) const ;
            inline bool operator!=(const rotation_velocity_3d & second) const { return !operator==(second); }

            /**
             * \brief Get the yaw rotation velocity
             * \note Alias for \ref rotation_velocity_2d::get_rotation_velocity
             * \return yaw rotation velocity or 0 if not set
             */
            inline rotation_velocity_t get_yaw_velocity() const { return get_rotation_velocity(); }

            /**
             * \brief Set the yaw rotation velocity
             * \note Alias for \ref rotation_velocity_2d::set_rotation_velocity
             * \param yaw_velocity - yaw rotation velocity to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-rotation
             * \return previous setting
             */
            inline rotation_velocity_t set_yaw_velocity(const rotation_velocity_t yaw_velocity){ return set_rotation_velocity(yaw_velocity); }


            /**
             * \brief Get the pitch rotation velocity
             * \return yaw rotation velocity or 0 if not set
             */
            inline rotation_velocity_t get_pitch_velocity() const { return m_pitch_vel; }

            /**
             * Set the m_pitch velocity
             * \param pitch_velocity - m_pitch velocity in radians per second
             * \return previous setting
             */
            rotation_velocity_t set_pitch_velocity(const rotation_velocity_t pitch_velocity);

            /**
             * Get the m_roll velocity in radians per second
             * \return m_roll velocity or 0 if not set
             */
            inline rotation_velocity_t get_roll_velocity() const { return m_roll_vel; }

            /**
             * Set the m_roll velocity
             * \param roll_velocity - m_roll velocity in radians per second
             * \return previous setting
             */
            rotation_velocity_t set_roll_velocity(const rotation_velocity_t roll_velocity);

        protected:

            rotation_velocity_t m_pitch_vel;
            rotation_velocity_t m_roll_vel;

        }; // cls rotation_velocity_3d

        //! \brief Represents the overall movement acceleration, idependent on 2D or 3D space
        class movement_acceleration {
        public:

            /**
             * \brief Creates a new movement acceleration helper with given acceleration
             * \param acceleration - acceleration to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-acceleration "normalized acceleration"
             */
            movement_acceleration(const accel_t acceleration);

            //! \brief Creates a new, empty movement acceleration helper
            movement_acceleration();

            movement_acceleration & operator=(const movement_acceleration & second);
            bool operator== (const movement_acceleration & second) const ;
            inline bool operator!=(const movement_acceleration & second) const { return !operator==(second); }

            /**
             * \brief Gets the movement acceleration in the movement direction
             * \return movement acceleration if set, 0 if unset or set to 0
             */
            inline accel_t get_acceleration() const { return m_acceleration; }

            /**
             * \brief Sets the movement acceleration
             * \param accel - acceleration to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-acceleration "normalized acceleration"
             * \return Previous setting
             */
            accel_t set_acceleration(const accel_t accel);

        protected:
            
            accel_t m_acceleration;
            
        }; // cls movement_acceleration

        //! \brief Represents the overall rotation acceleration, idependent on 2D or 3D space
        class rotation_acceleration {
        public:

            /**
             * \brief Creates a new rotation acceleration helper with given acceleration
             * \param acceleration - rotation acceleration to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-acceleration "normalized acceleration"
             */
            rotation_acceleration(const rotation_accel_t acceleration);

            //! \brief Creates a new, empty rotation acceleration helper
            rotation_acceleration();

            rotation_acceleration & operator=(const rotation_acceleration & second);
            bool operator== (const rotation_acceleration & second) const ;
            inline bool operator!=(const rotation_acceleration & second) const { return !operator==(second); }

            /**
             * \brief Gets the rotation acceleration
             * \return rotation acceleration if set, 0 if unset or set to 0
             */
            inline accel_t get_rotation_acceleration() const { return m_rotation_acceleration; }

            /**
             * \brief Sets the rotation acceleration
             * \param accel - rotation acceleration to be set
             * \note For TUIO 2.0 draft (non)compliance, see \ref normalized-acceleration "normalized acceleration"
             * \return Previous setting
             */
            accel_t set_rotation_acceleration(const accel_t accel) ;

        protected:
            
            rotation_accel_t m_rotation_acceleration;
            
        }; // cls rotation_acceleration

        //! \brief A helper providing session id
        class contact_session {
        public:

            /**
             * \brief Creates a new session id helper
             * \param session_id - session id to be set
             */
            contact_session(const session_id_t session_id);

            //! \brief Creates a new, empty session id helper
            contact_session();

            contact_session & operator=(const contact_session & second);
            bool operator== (const contact_session & second) const ;
            inline bool operator!=(const contact_session & second) const { return !operator==(second); }

            /**
             * \brief Gets the session id
             * \return session id of the pointer
             */
            inline session_id_t get_session_id() const { return m_session_id; }

            /**
             * \brief Sets the session id
             * \param session_id - id that is unique for contact, can group several types of messages
             * \return previous setting
             */
            virtual session_id_t set_session_id(session_id_t session_id);

        protected:

            session_id_t m_session_id;

        }; // cls contact_session

        //! \brief A helper class for all objects that have type and user attributes (as in \ref libkerat::message::pointer)
        class contact_type_user {
        public:

            /**
             * \brief Create a new contact type and user helper
             * \param type - type to be set, see protocol defined constants below
             * \param user - user id to be set, see \ref UID_NOUSER constant below
             */
            contact_type_user(const type_id_t type, const user_id_t user);
            
            //! \brief Create a new contact type and user helper with \ref TYPEID_UNKNOWN and \ref UID_NOUSER
            contact_type_user();

            contact_type_user & operator = (const contact_type_user & second);
            bool operator == (const contact_type_user & second) const ;
            inline bool operator!=(const contact_type_user & second) const { return !operator==(second); }

            /**
             * \brief Sets the compiled type and user id.
             * \param tu_id - compiled type and user id to be set
             */
            void set_tu_id(const tu_id_t tu_id);

            /**
             * \brief Gets the type id
             * \return one of the protocol-defined constants below (TYPEID_*)
             * or custom value greater than \ref TYPEID_FREE_BASE
             */
            inline type_id_t get_type_id() const { return m_type_id; }

            /**
             * \brief Sets the type id
             * \param type_id - one of the protocol-defined constants below (TYPEID_*)
             * or custom value greater than \ref TYPEID_FREE_BASE
             * \return previous setting
             */
            type_id_t set_type_id(const type_id_t type_id);

            /**
             * \brief Gets the id of user
             * \return user id or 0 as default (\ref UID_NOUSER)
             */
            inline user_id_t get_user_id() const { return m_user_id; }

            /**
             * \brief Sets the user ID. User ID 0 is reserved for no user (\ref UID_NOUSER)
             * \param user_id - user id to set, 0 as default (\ref UID_NOUSER)
             * \return previous setting
             */
            user_id_t set_user_id(const user_id_t user_id);

            // Constants for type ID's as defined in TUIO 2.0 specs
            static const type_id_t TYPEID_UNKNOWN = 0;
            static const type_id_t TYPEID_RIGHT_INDEX_FINGER = 1;
            static const type_id_t TYPEID_RIGHT_MIDDLE_FINGER = 2;
            static const type_id_t TYPEID_RIGHT_RING_FINGER = 3;
            static const type_id_t TYPEID_RIGHT_LITTLE_FINGER = 4;
            static const type_id_t TYPEID_RIGHT_THUMB_FINGER = 5;
            static const type_id_t TYPEID_LEFT_INDEX_FINGER = 6;
            static const type_id_t TYPEID_LEFT_MIDDLE_FINGER = 7;
            static const type_id_t TYPEID_LEFT_RING_FINGER = 8;
            static const type_id_t TYPEID_LEFT_LITTLE_FINGER = 9;
            static const type_id_t TYPEID_LEFT_THUMB_FINGER = 10;

            static const type_id_t TYPEID_STYLUS = 11;
            static const type_id_t TYPEID_LASERPOINTER = 12;
            static const type_id_t TYPEID_MOUSE = 13;
            static const type_id_t TYPEID_TRACKBALL = 14;
            static const type_id_t TYPEID_JOYSTICK = 15;

            static const type_id_t TYPEID_RIGHT_HAND_POINTING = 21;
            static const type_id_t TYPEID_RIGHT_HAND_OPEN = 22;
            static const type_id_t TYPEID_RIGHT_HAND_CLOSED = 23;
            static const type_id_t TYPEID_LEFT_HAND_POINTING = 24;
            static const type_id_t TYPEID_LEFT_HAND_OPEN = 25;
            static const type_id_t TYPEID_LEFT_HAND_CLOSED = 26;
            static const type_id_t TYPEID_RIGHT_FOOT = 27;
            static const type_id_t TYPEID_LEFT_FOOT = 28;
            static const type_id_t TYPEID_HEAD = 29;
            static const type_id_t TYPEID_PERSON = 30;

            //! This is the first ID that the draft claims to be free
            static const type_id_t TYPEID_FREE_BASE = 100;

            //! The user id 0 is reserved for no-user state
            static const user_id_t UID_NOUSER = 0;

        protected:

            type_id_t m_type_id;
            user_id_t m_user_id;

        }; // cls component_type_user

        /**
         * \brief A helper class for all objects that have their own component id
         * 
         * Id that is unique for contact component, can group several components of the same message type and session id
         */
        class contact_component {
        public:
            
            //! \brief Creates a new, empty component id helper
            contact_component();
            
            //! \brief Creates a new component id helper from given component id
            contact_component(const component_id_t component);

            contact_component & operator=(const contact_component & second);
            bool operator == (const contact_component & second) const ;
            inline bool operator!=(const contact_component & second) const { return !operator==(second); }

            /**
             * \brief Gets the component id
             * \return componet id of the pointer or 0 if unset or default
             */
            inline component_id_t get_component_id() const { return m_component_id; }

            /**
             * \brief Sets the component id
             * \param component_id - non-negative component id - 0 serves as unset/default value
             * \return previous setting
             */
            component_id_t set_component_id(const component_id_t component_id);

        protected:

            component_id_t m_component_id;

        }; // cls contact_component

        /**
         * \brief An utility helper for all message classes that have both 2D
         * and 3D variants
         * 
         * For example, \ref message::pointer "pointer" message can be represented
         * by 3D (/tuio2/p3d) or 2D (/tuio2/ptr) TUIO messages.
         */
        class message_output_mode {
        public:

            //! \brief Bitfield-like enum for message variants generation
            typedef enum { 
                //! for 2D message variant
                OUTPUT_MODE_2D = 0x1, 
                //! for 3D message variant
                OUTPUT_MODE_3D = 0x2, 
                //! for both 2D & 3D message variants
                OUTPUT_MODE_BOTH = 0x3 
            } output_mode_t;

            //! Creates a new output mode with \ref OUTPUT_MODE_BOTH setting
            message_output_mode();

            //! Creates a new output mode helper with given mode
            message_output_mode(output_mode_t mode);

            message_output_mode & operator=(const message_output_mode & second);
            bool operator == (const message_output_mode & second) const ;
            inline bool operator!=(const message_output_mode & second) const { return !operator==(second); }

            /**
             * \brief Gets the message output mode
             * \return one of \ref OUTPUT_MODE_2D, \ref OUTPUT_MODE_3D, \ref OUTPUT_MODE_BOTH
             * if both message types shall be generated
             */
            inline output_mode_t get_message_output_mode() const { return m_output_mode; }

            /**
             * \brief Sets the message output mode
             * \param output_mode - one of \ref OUTPUT_MODE_2D, \ref OUTPUT_MODE_3D, \ref OUTPUT_MODE_BOTH
             * \return previous setting
             */
            output_mode_t set_message_output_mode(output_mode_t output_mode);

        protected:

            output_mode_t m_output_mode;

        }; // cls message_output_mode

        /**
         * \brief A helper interface holding the link topology for 
         * \ref message::link_association "link_association"
         * \ref message::linked_list_association "linked_list_association"
         * \ref message::linked_tree_association "linked_tree_association"
         * messages
         */
        class link_topology {
        public:
            
            /**
             * \brief Type of association
             * 
             * Each link association can be either of logical or physical nature.
             * Type of association is determined by boolean value. See respective
             * constants \ref LINK_PHYSICAL LINK_LOGICAL
             */
            typedef bool link_association_type_t;
            
            //! \brief Link I/O ports
            struct link_entry {
                /**
                 * \brief Creates a new empty link
                 */
                link_entry():output_port(0), input_port(0){ ; }
                
                /**
                 * \brief Create a new link with given ports
                 * \param in - input port <0; 65535>
                 * \param out - output port <0; 65535>
                 */
                link_entry(const link_port_t in, const link_port_t out):output_port(out), input_port(in){ ; }
                
                bool operator==(const link_entry & second) const throw() {
                    return (output_port == second.output_port)
                        && (input_port == second.input_port);
                }
                bool operator<(const link_entry & second) const throw() {
                    if (output_port < second.output_port){ 
                        return true; 
                    } else if (output_port > second.output_port){ 
                        return false; 
                    }
                    return input_port < second.input_port;
                }
                
                inline bool operator!=(const link_entry & second) const throw() { return !operator==(second); }
                
                link_port_t output_port;
                link_port_t input_port;
            };
            
            /**
             * \brief Link topology graph type. Node values are 
             * \ref libkerat::session_id_t "session id's", 
             * edges are \ref link_entry "link entry" records
             */
            typedef internals::graph<session_id_t, link_entry> internal_link_graph;
            
            //! \brief true denotes physical link association
            static const link_association_type_t LINK_PHYSICAL = true;

            //! \brief false denotes logical link association
            static const link_association_type_t LINK_LOGICAL = false;
            
            //! \brief Creates a new physical link topology helper
            link_topology();

            /**
             * \brief Creates a new link topology helper of given association type
             * \param type - type of association to be created
             */
            link_topology(const link_association_type_t type);
            
            /**
             * \brief Creates a new link topology helper from given type and graph
             * \param type - type of association to be created
             * \param graph - link topology graph to create from
             */
            link_topology(const link_association_type_t type, const internal_link_graph & graph);
            
            virtual ~link_topology();
            
            /**
             * \brief Gets the link topology graph of this association
             * \return const reference to the link topology graph
             */
            inline const internal_link_graph & get_link_graph() const {
                return m_link_graph;
            }

            /**
             * \brief Gets the link type
             * \return true if link type is physical, false if logical
             */
            inline link_association_type_t get_link_type() const { return m_link_type; }

            /**
             * \brief Sets the link type
             * \param link_type - either \ref LINK_PHYSICAL of \ref LINK_LOGICAL
             * \return previous setting
             * \see LINK_PHYSICAL LINK_LOGICAL
             */
            link_association_type_t set_link_type(const link_association_type_t link_type);
            
            bool operator==(const link_topology & second) const ;
            inline bool operator!=(const link_topology & second) const { return !operator==(second); }
            
        protected:

            /**
             * \brief Sets the link topology graph. 
             * \param links - link graph to be set
             * \return Previous setting
             */
            virtual internal_link_graph set_link_graph(const internal_link_graph & links);

            link_association_type_t m_link_type;
            internal_link_graph m_link_graph;
        
        }; // cls link_topology
        
        //! \brief A helper interface for messages whose content can be scaled but only relative to it's center (2D variant)
        class scalable_2d {
        public:
            /**
             * \brief Scale the content's x axis dimmensions by given factor relative to given center
             * \param factor - factor to scale by
             * \param center - point that is in center of expanse
             */
            virtual void scale_x(float factor, const point_2d & center) = 0;

            /**
             * \brief Scale the content's y axis dimmensions by given factor relative to given center
             * \param factor - factor to scale by
             * \param center - point that is in center of expanse
             */
            virtual void scale_y(float factor, const point_2d & center) = 0;
        };
        
        //! \brief A helper interface for messages whose content can be scaled but only relative to it's center (3D variant)
        class scalable_3d: public scalable_2d {
        public:
            /**
             * \brief Scale the content's z axis dimmensions by given factor relative to given center
             * \param factor - factor to scale by
             * \param center - point that is in center of expanse
             */
            virtual void scale_z(float factor, const point_3d & center) = 0;
        };
        
        //! \brief A helper interface for messages whose content can be scaled regardles of it's center (2D variant)
        class scalable_independent_2d {
        public:
            /**
             * \brief Scale the content's x axis dimmensions by given factor
             * \param factor - factor to scale by
             */
            virtual void scale_x(float factor) = 0;

            /**
             * \brief Scale the content's y axis dimmensions by given factor
             * \param factor - factor to scale by
             */
            virtual void scale_y(float factor) = 0;
        };
        
        //! \brief A helper interface for messages whose content can be scaled regardles of it's center (3D variant)
        class scalable_independent_3d: public scalable_independent_2d {
        public:
            /**
             * \brief Scale the content's y axis dimmensions by given factor
             * \param factor - factor to scale by
             */
            virtual void scale_z(float factor) = 0;
        };
        
        //! \brief A helper interface for messages which can be moved on the sensor (2D variant)
        class movable_2d {
        public:
            /**
             * \brief Move the message in x axis by given factor
             * \param factor - factor to move by
             */
            virtual void move_x(coord_t factor) = 0;

            /**
             * \brief Move the message in y axis by given factor
             * \param factor - factor to move by
             */
            virtual void move_y(coord_t factor) = 0;
        };

        //! \brief A helper interface for messages which can be moved on the sensor (3D variant)
        class movable_3d: public movable_2d {
        public:
            /**
             * \brief Move the message in z axis by given factor
             * \param factor - factor to move by
             */
            virtual void move_z(coord_t factor) = 0;
        };

        //! \brief A helper interface for messages which can be rotated on the sensor but only relative to some center point (2D variant)
        //! \note Corresponds to support for objects with coordinate system rotation
        class rotatable_cs_2d {
        public:
            /**
             * \brief Rotate the message's content by given angle in the XY plane and given center
             * \param angle - angle to rotate by (radians)
             * \param center - center to rotate around
             */
            virtual void rotate_by(angle_t angle, const point_2d & center) = 0;
        };

        //! \brief A helper interface for messages which can be rotated on the sensor but only relative to some center point (3D variant)
        //! \note Corresponds to support for objects with coordinate system rotation
        class rotatable_cs_3d: public rotatable_cs_2d {
        public:
            /**
             * \brief Rotate the message's content by given angle aroud the yaw axis and given center
             * \note Usually alias for \ref rotatable_2d::rotate_by
             * \param angle - angle to rotate by (radians)
             * \param center - center to rotate around
             */
            virtual void rotate_yaw(angle_t angle, const point_3d & center);

            /**
             * \brief Rotate the message's content by given angle aroud the pitch axis and given center
             * \param angle - angle to rotate by (radians)
             * \param center - center to rotate around
             */
            virtual void rotate_pitch(angle_t angle, const point_3d & center) = 0;

            /**
             * \brief Rotate the message's content by given angle aroud the roll axis and given center
             * \param angle - angle to rotate by (radians)
             * \param center - center to rotate around
             */
            virtual void rotate_roll(angle_t angle, const point_3d & center) = 0;
        };
        
        //! \brief A helper interface for messages which can be rotated on the sensor regardles of it's center (2D variant)
        class rotatable_independent_2d {
        public:
            /**
             * \brief Rotate the message's content by given angle in the XY plane
             * \param angle - angle to rotate by (radians)
             */
            virtual void rotate_by(angle_t angle) = 0;
        };

        //! \brief A helper interface for messages which can be rotated on the sensor regardles of it's center (3D variant)
        class rotatable_independent_3d: public rotatable_independent_2d {
        public:
            /**
             * \brief Rotate the message's content by given angle aroud the yaw axis
             * \note Usually alias for \ref rotatable_2d::rotate_by
             * \param angle - angle to rotate by (radians)
             */
            virtual void rotate_yaw(angle_t angle);

            /**
             * \brief Rotate the message's content by given angle aroud the pitch axis
             * \param angle - angle to rotate by (radians)
             */
            virtual void rotate_pitch(angle_t angle) = 0;

            /**
             * \brief Rotate the message's content by given angle aroud the yaw axis
             * \param angle - angle to rotate by (radians)
             */
            virtual void rotate_roll(angle_t angle) = 0;
        };
        
    } // ns helpers

    // corresponding utils
    /**
     * \brief Compute the distance between two points
     * \param pt1 - compute distance from this point
     * \param pt2 - compute distance to this point
     * \return non-negative distance
     */
    inline double distance(const libkerat::helpers::point_2d & pt1, const libkerat::helpers::point_2d & pt2){
        double tmp1 = (pt1.get_x() - pt2.get_x());
        double tmp2 = (pt1.get_y() - pt2.get_y());
        return std::sqrt(tmp1*tmp1 + tmp2*tmp2);
    }
    /**
     * \brief Compute the distance between two points
     * \param pt1 - compute distance from this point
     * \param pt2 - compute distance to this point
     * \return non-negative distance
     */
    inline double distance(const libkerat::helpers::point_3d & pt1, const libkerat::helpers::point_3d & pt2){
        double tmp1 = (pt1.get_x() - pt2.get_x());
        double tmp2 = (pt1.get_y() - pt2.get_y());
        double tmp3 = (pt1.get_z() - pt2.get_z());
        return std::sqrt(tmp1*tmp1 + tmp2*tmp2 + tmp3*tmp3);
    }

} // ns libkerat

// corresponding utilities & pretty prints

std::ostream & operator<<(std::ostream & output, const libkerat::helpers::point_2d & pt);
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::point_3d & pt);
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::velocity_2d & pt);
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::velocity_3d & pt);
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::angle_2d & pt);
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::angle_3d & pt);

#endif // KERAT_MESSAGE_COMPONENTS_HPP
