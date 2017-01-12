/**
 * \file      tuio_message_token.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-25 15:51 UTC+2
 * \copyright BSD
 */

#include <kerat/message_helpers.hpp>
#include <kerat/utils.hpp>
#include <cmath>

namespace libkerat {
    namespace helpers {

        // 2D point 
        
        //! \brief Creates a new point on given coordinates
        point_2d::point_2d(const coord_t x, const coord_t y)
            :m_x_pos(x),m_y_pos(y)
        { ; }

        //! \brief Creates a new point on coordinates [0, 0]
        point_2d::point_2d()
            :m_x_pos(0),m_y_pos(0)
        { ; }

        coord_t point_2d::set_x(const coord_t x) {
            coord_t oldval = m_x_pos;
            m_x_pos = x;
            return oldval;
        }

        coord_t point_2d::set_y(const coord_t y) {
            coord_t oldval = m_y_pos;
            m_y_pos = y;
            return oldval;
        }
        
        point_2d & point_2d::operator=(const point_2d & second){ 
            m_x_pos = second.m_x_pos; 
            m_y_pos = second.m_y_pos; 
            return *this; 
        }
        
        bool point_2d::operator== (const point_2d & second) const { 
            return (m_x_pos == second.m_x_pos) 
                && (m_y_pos == second.m_y_pos); 
        }
        
        point_2d point_2d::operator+(const point_2d & second) const { 
            return point_2d(
                get_x() + second.get_x(), 
                get_y() + second.get_y()
            ); 
        }
        
        point_2d point_2d::operator-(const point_2d & second) const { 
            return point_2d(
                get_x() - second.get_x(), 
                get_y() - second.get_y()
            ); 
        }

        point_2d point_2d::operator*(const coord_t factor) const { 
            return point_2d(
                get_x()*factor, 
                get_y()*factor
            ); 
        }

        point_2d point_2d::operator/(const coord_t factor) const { 
            return point_2d(
                get_x()/factor, 
                get_y()/factor
            ); 
        }
        
        point_2d & point_2d::operator+=(const point_2d & second){ 
            set_x(get_x() + second.get_x()); 
            set_y(get_y() + second.get_y()); 
            return *this; 
        }
        
        point_2d & point_2d::operator-=(const point_2d & second){ 
            set_x(get_x() - second.get_x()); 
            set_y(get_y() - second.get_y()); 
            return *this; 
        }
        
        point_2d & point_2d::operator*=(const coord_t factor){
            set_x(get_x()*factor); 
            set_y(get_y()*factor); 
            return *this; 
        }

        point_2d & point_2d::operator/=(const coord_t factor){
            set_x(get_x()/factor); 
            set_y(get_y()/factor); 
            return *this; 
        }

        bool point_2d::operator<(const point_2d & second) const {
            if (get_x() < second.get_x()){
                return true;
            } else if (get_x() == second.get_x()){
                return get_y() < second.get_y();
            }
            return false;
        }

        // 3D point 
        point_3d::point_3d(const coord_t x, const coord_t y, const coord_t z)
            :point_2d(x, y),m_z_pos(z)
        { ; }
        
        point_3d::point_3d(const point_2d & pt)
            :point_2d(pt), m_z_pos(0)
        { ; }

        point_3d::point_3d()
            :m_z_pos(0)
        { ; }

        point_3d & point_3d::operator=(const point_3d & second){ 
            point_2d::operator=(second); 
            m_z_pos = second.m_z_pos; 
            return *this; 
        }
        
        bool point_3d::operator== (const point_3d & second) const { 
            return point_2d::operator==(second) 
                && (m_z_pos == second.m_z_pos); 
        }
        
        point_3d point_3d::operator+(const point_3d & second) const { 
            return point_3d(
                get_x() + second.get_x(), 
                get_y() + second.get_y(), 
                get_z() + second.get_z()
            ); 
        }
        
        point_3d point_3d::operator-(const point_3d & second) const { 
            return point_3d(
                get_x() - second.get_x(), 
                get_y() - second.get_y(), 
                get_z() - second.get_z()
            ); 
        }

        point_3d point_3d::operator*(const coord_t factor) const { 
            return point_3d(
                get_x()*factor, 
                get_y()*factor, 
                get_z()*factor
            ); 
        }

        point_3d point_3d::operator/(const coord_t factor) const { 
            return point_3d(
                get_x()/factor, 
                get_y()/factor, 
                get_z()/factor
            ); 
        }

        point_3d & point_3d::operator+=(const point_3d & second){ 
            point_2d::operator+=(second);
            set_z(get_z() + second.get_z()); 
            return *this; 
        }
        
        point_3d & point_3d::operator-=(const point_3d & second){ 
            point_2d::operator-=(second);
            set_z(get_z() - second.get_z()); 
            return *this; 
        }

        point_3d & point_3d::operator*=(const coord_t factor){ 
            point_2d::operator*=(factor);
            set_z(get_z()*factor); 
            return *this; 
        }

        point_3d & point_3d::operator/=(const coord_t factor){ 
            point_2d::operator/=(factor);
            set_z(get_z()/factor); 
            return *this; 
        }

        bool point_3d::operator<(const point_3d & second) const {
            if (get_x() < second.get_x()){
                return true;
            } else if (get_x() == second.get_x()){
                if (get_y() < second.get_y()){
                    return true;
                } else if (get_y() == second.get_y()){
                    return get_z() < second.get_z();
                }
            }
            return false;
        }        
        
        coord_t point_3d::set_z(const coord_t z) {
            coord_t oldval = m_z_pos;
            m_z_pos = z;
            return oldval;
        }
    
        // 2D velocity
        velocity_2d::velocity_2d(const velocity_t x_velocity, const velocity_t y_velocity)
            :m_x_vel(x_velocity),m_y_vel(y_velocity)
        { ; }

        velocity_2d::velocity_2d()
            :m_x_vel(0),m_y_vel(0)
        { ; }

        velocity_2d & velocity_2d::operator=(const velocity_2d & second){ 
            m_x_vel = second.m_x_vel; 
            m_y_vel = second.m_y_vel;
            return *this; 
        }
        
        bool velocity_2d::operator== (const velocity_2d & second) const { 
            return (m_x_vel == second.m_x_vel) 
                && (m_y_vel == second.m_y_vel); 
        }
        
        velocity_t velocity_2d::set_x_velocity(velocity_t x_velocity) {
            velocity_t oldval = m_x_vel;
            m_x_vel = x_velocity;
            return oldval;
        }

        velocity_t velocity_2d::set_y_velocity(velocity_t y_velocity) {
            velocity_t oldval = m_y_vel;
            m_y_vel = y_velocity;
            return oldval;
        }
        
        bool velocity_2d::has_velocity() const {
            return (m_x_vel != 0)
                || (m_y_vel != 0);
        }
        
        velocity_t velocity_2d::get_overall_velocity() const {
            return sqrt(
                (m_x_vel * m_x_vel)
                + (m_y_vel * m_y_vel)
            );
        }

        // 3D velocity
        velocity_3d::velocity_3d()
            :velocity_2d(0, 0),m_z_vel(0)
        { ; }

        velocity_3d::velocity_3d(const velocity_t x_velocity, const velocity_t y_velocity, const velocity_t z_velocity)
            :velocity_2d(x_velocity, y_velocity),m_z_vel(z_velocity)
        { ; }

        velocity_3d & velocity_3d::operator=(const velocity_3d & second){ 
            velocity_2d::operator=(second); 
            m_z_vel = second.m_z_vel; 
            return *this; 
        }
        
        bool velocity_3d::operator== (const velocity_3d & second) const { 
            return velocity_2d::operator==(second) 
                && (m_z_vel == second.m_z_vel); 
        }
        
        velocity_t velocity_3d::set_z_velocity(velocity_t z_velocity) {
            velocity_t oldval = m_z_vel;
            m_z_vel = z_velocity;
            return oldval;
        }
        
        bool velocity_3d::has_velocity() const {
            return velocity_2d::has_velocity()
                || (m_z_vel != 0);
        }
        
        velocity_t velocity_3d::get_overall_velocity() const {
            return sqrt(
                (m_x_vel * m_x_vel)
                + (m_y_vel * m_y_vel)
                + (m_z_vel * m_z_vel)
            );
        }
        
        // 2D angle
        angle_2d::angle_2d()
            :m_yaw(0)
        { ; }

        angle_2d::angle_2d(const angle_t angle)
            :m_yaw(angle)
        { ; }

        angle_2d & angle_2d::operator=(const angle_2d & second){ 
            m_yaw = second.m_yaw; 
            return *this; 
        }
        
        bool angle_2d::operator== (const angle_2d & second) const { 
            return (m_yaw == second.m_yaw); 
        }
        
        angle_t angle_2d::set_angle(const angle_t angle) {
            angle_t retval = m_yaw;
            m_yaw = angle;
            return retval;
        }
        
        // 3D angle
        angle_3d::angle_3d(const angle_t yaw, const angle_t pitch, const angle_t roll)
            :angle_2d(yaw),m_pitch(pitch),m_roll(roll)
        { ; }

        angle_3d::angle_3d()
            :angle_2d(0),m_pitch(0),m_roll(0)
        { ; }

        angle_3d & angle_3d::operator=(const angle_3d & second){ angle_2d::operator=(second); m_pitch = second.m_pitch; m_roll = second.m_roll; return *this; }
        bool angle_3d::operator== (const angle_3d & second) const { return angle_2d::operator==(second) && (m_pitch == second.m_pitch) && (m_roll == second.m_roll); }

        angle_t angle_3d::set_pitch(const angle_t pitch){
            angle_t retval = m_pitch;
            m_pitch = pitch;
            return retval;
        }
        angle_t angle_3d::set_roll(const angle_t roll){
            angle_t retval = m_roll;
            m_roll = roll;
            return retval;
        }
        
        // 2D rotation velocity
        rotation_velocity_2d::rotation_velocity_2d(const rotation_velocity_t yaw_velocity)
            :m_yaw_vel(yaw_velocity)
        { ; }
        
        rotation_velocity_2d::rotation_velocity_2d()
            :m_yaw_vel(0)
        { ; }

        rotation_velocity_2d & rotation_velocity_2d::operator=(const rotation_velocity_2d & second){
            m_yaw_vel = second.m_yaw_vel;
            return *this;
        }
        
        bool rotation_velocity_2d::operator== (const rotation_velocity_2d & second) const { 
            return (m_yaw_vel == second.m_yaw_vel);
        }
        
        rotation_velocity_t rotation_velocity_2d::set_rotation_velocity(const rotation_velocity_t rotation_velocity){
            rotation_velocity_t retval = m_yaw_vel;
            m_yaw_vel = rotation_velocity;
            return retval;
        }
        
        // 3D rotation velocity
        rotation_velocity_3d::rotation_velocity_3d()
            :rotation_velocity_2d(0),m_pitch_vel(0),m_roll_vel(0)
        { ; }

        rotation_velocity_3d::rotation_velocity_3d(const rotation_velocity_t yaw_velocity, const rotation_velocity_t pitch_velocity, const rotation_velocity_t roll_velocity)
            :rotation_velocity_2d(yaw_velocity),m_pitch_vel(pitch_velocity),m_roll_vel(roll_velocity)
        { ; }

        rotation_velocity_3d & rotation_velocity_3d::operator=(const rotation_velocity_3d & second){
            rotation_velocity_2d::operator=(second); 
            m_pitch_vel = second.m_pitch_vel; 
            m_roll_vel = second.m_roll_vel; 
            return *this; 
        }
        
        bool rotation_velocity_3d::operator== (const rotation_velocity_3d & second) const { 
            return rotation_velocity_2d::operator==(second) 
                && (m_pitch_vel == second.m_pitch_vel) 
                && (m_roll_vel == second.m_roll_vel); 
        }
        
        rotation_velocity_t rotation_velocity_3d::set_pitch_velocity(const rotation_velocity_t pitch_velocity){
            rotation_velocity_t retval = m_pitch_vel;
            m_pitch_vel = pitch_velocity;
            return retval;
        }

        rotation_velocity_t rotation_velocity_3d::set_roll_velocity(const rotation_velocity_t roll_velocity){
            rotation_velocity_t retval = m_roll_vel;
            m_roll_vel = roll_velocity;
            return retval;
        }
        
        // movement acceleration
        movement_acceleration::movement_acceleration(const accel_t acceleration)
            :m_acceleration(acceleration)
        { ; }
        
        movement_acceleration::movement_acceleration()
            :m_acceleration(0)
        { ; }

        movement_acceleration & movement_acceleration::operator=(const movement_acceleration & second){
            m_acceleration = second.m_acceleration;
            return *this;
        }
        
        bool movement_acceleration::operator== (const movement_acceleration & second) const {
            return (m_acceleration == second.m_acceleration);
        }
        
        accel_t movement_acceleration::set_acceleration(const accel_t accel) {
            accel_t oldval = m_acceleration;
            m_acceleration = accel;
            return oldval;
        }
        
        // rotation acceleration
        rotation_acceleration::rotation_acceleration(const rotation_accel_t acceleration)
            :m_rotation_acceleration(acceleration)
        { ; }

        rotation_acceleration::rotation_acceleration()
            :m_rotation_acceleration(0)
        { ; }

        rotation_acceleration & rotation_acceleration::operator=(const rotation_acceleration & second){ 
            m_rotation_acceleration = second.m_rotation_acceleration; 
            return *this; 
        }

        bool rotation_acceleration::operator== (const rotation_acceleration & second) const { 
            return (m_rotation_acceleration == second.m_rotation_acceleration);
        }

        accel_t rotation_acceleration::set_rotation_acceleration(const accel_t accel) {
            accel_t oldval = m_rotation_acceleration;
            m_rotation_acceleration = accel;
            return oldval;
        }
        
        // session
        contact_session::contact_session(const session_id_t session_id)
            :m_session_id(session_id)
        { ; }
        
        contact_session::contact_session()
             :m_session_id(0)
        { ; }

        contact_session & contact_session::operator=(const contact_session & second) { 
            m_session_id = second.m_session_id; 
            return *this; 
        }
        
        bool contact_session::operator== (const contact_session & second) const { 
            return m_session_id == second.m_session_id; 
        }
        
        session_id_t contact_session::set_session_id(session_id_t session_id) {
            session_id_t oldval = m_session_id;
            m_session_id = session_id;
            return oldval;
        }
        
        // contact type user

        contact_type_user::contact_type_user(const type_id_t type, const user_id_t user)
            :m_type_id(type), m_user_id(user)
        { ; }

        contact_type_user::contact_type_user()
            :m_type_id(TYPEID_UNKNOWN), m_user_id(UID_NOUSER)
        { ; }

        contact_type_user & contact_type_user::operator = (const contact_type_user & second){ 
            m_type_id = second.m_type_id; 
            m_user_id = second.m_user_id; 
            return *this; 
        }
        
        bool contact_type_user::operator == (const contact_type_user & second) const { 
            return (m_type_id == second.m_type_id) 
                && (m_user_id == second.m_user_id); 
        }
        
        void contact_type_user::set_tu_id(const tu_id_t tu_id) { 
            libkerat::decompile_tuid(tu_id, m_type_id, m_user_id);
        }

        type_id_t contact_type_user::set_type_id(const type_id_t type_id) {
            type_id_t oldval = m_type_id;
            m_type_id = type_id;
            return oldval;
        }

        user_id_t contact_type_user::set_user_id(const user_id_t user_id) {
            user_id_t oldval = m_user_id;
            m_user_id = user_id;
            return oldval;
        }
        
        // component
        contact_component::contact_component()
            :m_component_id(0)
        { ; }

        contact_component::contact_component(const component_id_t component)
            :m_component_id(component)
        { ; }

        contact_component & contact_component::operator=(const contact_component & second) {
            m_component_id = second.m_component_id; 
            return *this; 
        }
        
        bool contact_component::operator == (const contact_component & second) const { 
            return m_component_id == second.m_component_id; 
        }
        
        component_id_t contact_component::set_component_id(const component_id_t component_id) {
            component_id_t oldval = m_component_id;
            m_component_id = component_id;
            return oldval;
        }

        // message output mode
        message_output_mode::message_output_mode()
            :m_output_mode(OUTPUT_MODE_BOTH)
        { ; }

        message_output_mode::message_output_mode(output_mode_t mode)
            :m_output_mode(mode)
        { ; }

        message_output_mode & message_output_mode::operator=(const message_output_mode & second) { 
            m_output_mode = second.m_output_mode; 
            return *this; 
        }
        
        bool message_output_mode::operator == (const message_output_mode & second) const { 
            return m_output_mode == second.m_output_mode; 
        }
        
        message_output_mode::output_mode_t message_output_mode::set_message_output_mode(output_mode_t output_mode) {
            output_mode_t oldval = m_output_mode;
            m_output_mode = output_mode;
            return oldval;
        }

        // link topology
        link_topology::link_topology()
            :m_link_type(LINK_PHYSICAL)
        { ; }

        link_topology::link_topology(const link_association_type_t type)
            :m_link_type(type)
        { ; }

        link_topology::link_topology(const link_association_type_t type, const internal_link_graph & graph)
            :m_link_type(type)
        {
            set_link_graph(graph);
        }

        link_topology::~link_topology(){
            ;
        }

        link_topology::link_association_type_t link_topology::set_link_type(const link_topology::link_association_type_t link_type) {
            link_association_type_t oldval = m_link_type;
            m_link_type = link_type;
            return oldval;
        }

        link_topology::internal_link_graph link_topology::set_link_graph(const link_topology::internal_link_graph & links){
            internal_link_graph oldval = m_link_graph;
            m_link_graph = links;
            return oldval;
        }

        bool link_topology::operator==(const link_topology & topology) const {
            return (
                (m_link_type == topology.m_link_type)
                && (m_link_graph == topology.m_link_graph)
            );
        }

        void rotatable_independent_3d::rotate_yaw(angle_t angle){ return rotate_by(angle); }
        void rotatable_cs_3d::rotate_yaw(angle_t angle, const point_3d & center){ return rotate_by(angle, center); }
        
    } // ns helpers
} // ns libkerat


/**
 * \brief Pretty-print the point
 * \param output - output stream to print the point into
 * \param pt - point to be printed out
 * \return the output stream for sequencing
 * \note Format: <tt>[{x}, {y}]</tt>
 */
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::point_2d & pt){
    output << "[" << pt.get_x() << ", " << pt.get_y() << "]";
    return output;
}

/**
 * \brief Pretty-print the point
 * \param output - output stream to print the point into
 * \param pt - point to be printed out
 * \return the output stream for sequencing
 * \note Format: <tt>[{x}, {y}, {z}]</tt>
 */
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::point_3d & pt){
    output << "[" << pt.get_x() << ", " << pt.get_y() << ", " << pt.get_z() << "]";
    return output;
}

/**
 * \brief Pretty-print the velocity
 * \param output - output stream to print the point into
 * \param vel - velocity to be printed out
 * \return the output stream for sequencing
 * \note Format: <tt>({x velocity}, {y velocity})</tt>
 */
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::velocity_2d & vel){
    output << "(" << vel.get_x_velocity() << ", " << vel.get_y_velocity() << ")";
    return output;
}

/**
 * \brief Pretty-print the velocity
 * \param output - output stream to print the point into
 * \param vel - velocity to be printed out
 * \return the output stream for sequencing
 * \note Format: <tt>({x velocity}, {y velocity}, {z velocity})</tt>
 */
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::velocity_3d & vel){
    output << "(" << vel.get_x_velocity() << ", " << vel.get_y_velocity() << ", " << vel.get_z_velocity() << ")";
    return output;
}

/**
 * \brief Pretty-print the angle
 * \param output - output stream to print the point into
 * \param angle - angle to be printed out
 * \return the output stream for sequencing
 * \note Format: <tt>|&lt;[{angle}]</tt>
 */
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::angle_2d & angle){
    output << "|<[" << angle.get_angle() << "]";
    return output;
}

/**
 * \brief Pretty-print the angle
 * \param output - output stream to print the point into
 * \param angle - angle to be printed out
 * \return the output stream for sequencing
 * \note Format: <tt>|&lt;[{yaw}, {pitch}, {roll}]</tt>
 */
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::angle_3d & angle){
    output << "|<[" << angle.get_yaw() << ", " << angle.get_pitch() << ", " << angle.get_roll() << "]";
    return output;
}

/**
 * \brief Pretty-print the rotation velocity
 * \param output - output stream to print the point into
 * \param velocity - rotation velocity to be printed out
 * \return the output stream for sequencing
 * \note Format: <tt>|&lt;({angle})</tt>
 */
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::rotation_velocity_2d & velocity){
    output << "|<(" << velocity.get_rotation_velocity() << ")";
    return output;
}

/**
 * \brief Pretty-print the rotation velocity
 * \param output - output stream to print the point into
 * \param velocity - rotation velocity to be printed out
 * \return the output stream for sequencing
 * \note Format: <tt>|&lt;({yaw}, {pitch}, {roll})</tt>
 */
std::ostream & operator<<(std::ostream & output, const libkerat::helpers::rotation_velocity_3d & velocity){
    output << "|<(" << velocity.get_yaw_velocity() << ", " << velocity.get_pitch_velocity() << ", " << velocity.get_roll_velocity() << ")";
    return output;
}
