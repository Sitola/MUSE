/**
 * \file      tuio_message_frame.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-26 18:51 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_frame.hpp>
#include <kerat/utils.hpp>
#include <lo/lo.h>
#include <list>
#include <vector>
#include <cstring>

namespace libkerat {

    namespace message {

        const char * frame::PATH = "/tuio2/frm";

        frame::frame(frame_id_t frame_id):m_f_id(frame_id),m_addr(0),m_inst(0),m_dim_x(0),m_dim_y(0){
            lo_timetag_now(&m_time);
        }

        frame::frame(frame_id_t frame_id, timetag_t timestamp):m_f_id(frame_id),m_time(timestamp),m_addr(0),m_inst(0),m_dim_x(0),m_dim_y(0){
            ;
        }

        frame::frame(frame_id_t frame_id, timetag_t timestamp, std::string appname,
            addr_ipv4_t address, instance_id_t instance, dimmension_t sensor_width, dimmension_t sensor_height)
            :m_f_id(frame_id), m_time(timestamp),
            m_app(appname), m_addr(address), m_inst(instance),
            m_dim_x(sensor_width), m_dim_y(sensor_height)
        { ; }

        frame::frame(const frame & orig){
            m_f_id = orig.m_f_id;
            m_time = orig.m_time;
            m_app = orig.m_app;
            m_addr = orig.m_addr;
            m_inst = orig.m_inst;
            m_dim_x = orig.m_dim_x;
            m_dim_y = orig.m_dim_y;
        }

        frame & frame::operator=(const frame & orig){

            m_f_id = orig.m_f_id;
            m_time = orig.m_time;
            m_app = orig.m_app;
            m_addr = orig.m_addr;
            m_inst = orig.m_inst;
            m_dim_x = orig.m_dim_x;
            m_dim_y = orig.m_dim_y;

            return *this;
        }

        bool frame::operator ==(const frame& second) const {
            return (
                (m_f_id  == second.m_f_id) &&
                (m_time  == second.m_time) &&
                (m_app   == second.m_app) &&
                (m_addr  == second.m_addr) &&
                (m_inst  == second.m_inst) &&
                (m_dim_x == second.m_dim_x) &&
                (m_dim_y == second.m_dim_y)
            );
        }

        kerat_message * frame::clone() const { return new frame(*this); }

        void frame::print(std::ostream & output) const { output << *this; }

        bool frame::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            bool iext = is_extended();
            dimmensions_t dim = compile_dimmensions(m_dim_x, m_dim_y);

            // short
                lo_message msg_frame = lo_message_new();
                lo_message_add_int32(msg_frame, m_f_id);
                lo_message_add_timetag(msg_frame, m_time);

                if (iext){
                    lo_message_add_string(msg_frame, m_app.c_str());
                    lo_message_add_int32(msg_frame, m_addr);
                    lo_message_add_int32(msg_frame, m_inst);
                    lo_message_add_int32(msg_frame, dim);
                }
                lo_bundle_add_message(target, PATH, msg_frame);

            return true;

        }

        addr_ipv4_t frame::set_address(addr_ipv4_t address) {
            addr_ipv4_t oldval = m_addr;
            m_addr = address;
            return oldval;
        }

        std::string frame::set_app_name(std::string appname) {
            std::string oldval = m_app;
            m_app = appname;
            return oldval;
        }

        dimmension_t frame::set_sensor_width(dimmension_t sensor_width) {
            dimmension_t oldval = m_dim_x;
            m_dim_x = sensor_width;
            return oldval;
        }

        dimmensions_t frame::set_sensor_dim(dimmensions_t dim) {
            dimmensions_t retval = compile_dimmensions(m_dim_x, m_dim_y);
            decompile_dimmensions(dim, m_dim_x, m_dim_y);
            return retval;
        }

        dimmension_t frame::set_sensor_height(dimmension_t sensor_height) {
            dimmension_t oldval = m_dim_y;
            m_dim_y = sensor_height;
            return oldval;
        }

        frame_id_t frame::set_frame_id(frame_id_t frame_id) {
            frame_id_t oldval = m_f_id;
            m_f_id = frame_id;
            return oldval;
        }

        instance_id_t frame::set_instance(instance_id_t instance) {
            instance_id_t oldval = m_inst;
            m_inst = instance;
            return oldval;
        }

        timetag_t frame::set_timestamp(timetag_t timestamp) {
            timetag_t oldval = m_time;
            m_time = timestamp;
            return oldval;
        }

        bool frame::is_extended() const {
            return (
                (!get_app_name().empty())
                || (get_address() != 0)
                || (get_instance() != 0)
                || (get_sensor_width() != 0)
                || (get_sensor_height() != 0)
            );
        }



    } // ns message

    namespace internals { namespace parsers {
        /**
         * \brief TUIO 2.0 frame (/tuio2/frm) parser
         *
         * The parserd messages are returned through the results argument. To
         * understand the remaining arguments, see
         * \ref libkerat::simple_client::message_convertor_entry
         *
         * \return true if the message was sucessfully recognized, false if an
         * error has occured or was not recognized
         *
         * \see libkerat::simple_client
         * libkerat::simple_client::message_convertor_entry
         * libkerat::kerat_message
         */
        bool parse_frm(
            std::vector<libkerat::kerat_message *> & results,
            const char * path,
            const char * types,
            lo_arg ** argv,
            int argc,
            void * user_data __attribute__((unused))
        ){
            if (strcmp(path, libkerat::message::frame::PATH) != 0){ return false; }
            if (argc < 2){ return false; }
            if (strncmp("it", types, 2) != 0){ return false; }

            libkerat::message::frame * msg_frame = new libkerat::message::frame(argv[0]->i, argv[1]->t);

            if (argc == 6){
                if (strncmp("siii", types+2, 4) == 0){
                    msg_frame->set_app_name(&(argv[2]->s));
                    msg_frame->set_address(argv[3]->i);
                    msg_frame->set_instance(argv[4]->i);
                    msg_frame->set_sensor_dim(argv[5]->i);
                } else {
                    delete msg_frame;
                    msg_frame = NULL;
                    return false;
                }
            }

            results.push_back(msg_frame);
            return true;

        }
    }} // ns parsers // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::frame "frame" message to given output stream
 * \note Format: <tt>
 * {OSC path} {frame id} \\{{timestamp}\\}[ {application name} {ip address} {instance} \\[{sensor width}, {sensor height}\\]]
 * </tt>
 * \param output - output stream to write to
 * \param msg_frm - data message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::frame & msg_frm){
    output << libkerat::message::frame::PATH << " " << msg_frm.get_frame_id()
        << " {" << msg_frm.get_timestamp().sec << ", " << msg_frm.get_timestamp().frac << "}";

    if (msg_frm.is_extended()){
        libkerat::addr_ipv4_t ip = msg_frm.get_address();
        output << " \"" << msg_frm.get_app_name() << "\" "
            << ((ip >> 24) & 0xFF) << "." << ((ip >> 16) & 0xFF) << "." << ((ip >> 8) & 0xFF) << "." << (ip & 0xFF)
            << " " << msg_frm.get_instance()
            << " [" << msg_frm.get_sensor_width() << ", " << msg_frm.get_sensor_height() << "]";
    }

    return output;
}
