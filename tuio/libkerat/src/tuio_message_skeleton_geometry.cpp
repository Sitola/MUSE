/**
 * \file      tuio_message_skeleton_geometry.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-03-15 23:18 UTC+1
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/tuio_message_skeleton_geometry.hpp>
#include <kerat/utils.hpp>
#include <cstring>
#include <lo/lo.h>
#include <algorithm>
#include <cassert>
#include <vector>

// empty edge writer
struct skeleton_edge_empty_writer {
    void operator()(const libkerat::message::skeleton::skeleton_graph::edge_handle & handle __attribute__((unused))){ ; }
};
// empty rollback writer
struct skeleton_rollback_empty_writer {
    void operator()(
        const libkerat::message::skeleton::skeleton_graph::node_handle & handle1 __attribute__((unused)),
        const libkerat::message::skeleton::skeleton_graph::node_handle & handle2 __attribute__((unused)),
        size_t rollbacks_count __attribute__((unused))
    ){ ; }
};

template <typename POINT>
struct skeleton_node_lo_writer {
    skeleton_node_lo_writer(lo_message msg):m_message(msg){ ; }
    void operator()(const libkerat::message::skeleton::skeleton_graph::node_handle & handle);

    lo_message m_message;
};
template <> void skeleton_node_lo_writer<libkerat::helpers::point_2d>::operator ()(
    const libkerat::message::skeleton::skeleton_graph::node_handle & handle
){
    libkerat::helpers::point_2d pt = handle.get_value();
    lo_message_add_float(m_message, pt.get_x());
    lo_message_add_float(m_message, pt.get_y());
}
template <> void skeleton_node_lo_writer<libkerat::helpers::point_3d>::operator ()(
    const libkerat::message::skeleton::skeleton_graph::node_handle & handle
){
    libkerat::helpers::point_3d pt = handle.get_value();
    lo_message_add_float(m_message, pt.get_x());
    lo_message_add_float(m_message, pt.get_y());
    lo_message_add_float(m_message, pt.get_z());
}

struct skeleton_rollback_lo_writer {
    skeleton_rollback_lo_writer(lo_message msg):m_message(msg){ ; }
    void operator()(
        const libkerat::message::skeleton::skeleton_graph::node_handle & handle1 __attribute__((unused)),
        const libkerat::message::skeleton::skeleton_graph::node_handle & handle2 __attribute__((unused)),
        size_t rollbacks_count
    ){
        lo_message_add_int32(m_message, rollbacks_count);
    }

    lo_message m_message;
};

template <typename POINT>
struct skeleton_node_ostream_writer {

    skeleton_node_ostream_writer(std::ostream & output):m_output(output){ ; }
    void operator()(const libkerat::message::skeleton::skeleton_graph::node_handle & handle){
        m_output << (const POINT)handle.get_value();
    }

    std::ostream & m_output;
};
struct skeleton_rollback_ostream_writer {
    skeleton_rollback_ostream_writer(std::ostream & output):m_output(output){ ; }
    void operator()(
        const libkerat::message::skeleton::skeleton_graph::node_handle & handle1 __attribute__((unused)),
        const libkerat::message::skeleton::skeleton_graph::node_handle & handle2 __attribute__((unused)),
        size_t rollbacks_count
    ){
        m_output << " " << rollbacks_count;
    }

    std::ostream & m_output;
};

struct skeleton_node_scaler_x {
    skeleton_node_scaler_x(const float factor, const libkerat::helpers::point_3d & center)
        :m_factor(factor),m_center(center)
    { ; }

    void operator()(libkerat::message::skeleton::skeleton_graph::node_handle handle){
        libkerat::helpers::point_3d & pt = handle.get_value();
        pt.set_x(m_center.get_x() + m_factor*(pt.get_x()-m_center.get_x()));
    }

    const float m_factor;
    const libkerat::helpers::point_3d & m_center;
};
struct skeleton_node_scaler_y {
    skeleton_node_scaler_y(const float factor, const libkerat::helpers::point_3d & center)
        :m_factor(factor),m_center(center)
    { ; }

    void operator()(libkerat::message::skeleton::skeleton_graph::node_handle handle){
        libkerat::helpers::point_3d & pt = handle.get_value();
        pt.set_y(m_center.get_y() + m_factor*(pt.get_y()-m_center.get_y()));
    }

    const float m_factor;
    const libkerat::helpers::point_3d & m_center;
};
struct skeleton_node_scaler_z {
    skeleton_node_scaler_z(const float factor, const libkerat::helpers::point_3d & center)
        :m_factor(factor),m_center(center)
    { ; }

    void operator()(libkerat::message::skeleton::skeleton_graph::node_handle handle){
        libkerat::helpers::point_3d & pt = handle.get_value();
        pt.set_z(m_center.get_z() + m_factor*(pt.get_z()-m_center.get_z()));
    }

    const float m_factor;
    const libkerat::helpers::point_3d & m_center;
};

struct skeleton_node_mover_x {
    skeleton_node_mover_x(const libkerat::coord_t factor):m_factor(factor){ ; }

    void operator()(libkerat::message::skeleton::skeleton_graph::node_handle handle){
        libkerat::helpers::point_3d & pt = handle.get_value();
        pt.set_x(pt.get_x() + m_factor);
    }

    const libkerat::coord_t m_factor;
};
struct skeleton_node_mover_y {
    skeleton_node_mover_y(const libkerat::coord_t factor):m_factor(factor){ ; }

    void operator()(libkerat::message::skeleton::skeleton_graph::node_handle handle){
        libkerat::helpers::point_3d & pt = handle.get_value();
        pt.set_y(pt.get_y() + m_factor);
    }

    const libkerat::coord_t m_factor;
};
struct skeleton_node_mover_z {
    skeleton_node_mover_z(const libkerat::coord_t factor):m_factor(factor){ ; }

    void operator()(libkerat::message::skeleton::skeleton_graph::node_handle handle){
        libkerat::helpers::point_3d & pt = handle.get_value();
        pt.set_z(pt.get_z() + m_factor);
    }

    const libkerat::coord_t m_factor;
};

struct skeleton_node_rotator {
    typedef void (*callback)(libkerat::helpers::point_3d&, const libkerat::helpers::point_3d&, libkerat::angle_t angle);

    skeleton_node_rotator(const libkerat::angle_t angle, const libkerat::helpers::point_3d & center, callback f)
        :m_angle(angle),m_center(center), m_f(f)
    { ; }

    void operator()(libkerat::message::skeleton::skeleton_graph::node_handle handle) const {
        libkerat::helpers::point_3d & pt = handle.get_value();
        m_f(pt, m_center, m_angle);
    }

    const libkerat::angle_t m_angle;
    const libkerat::helpers::point_3d & m_center;
    callback m_f;
};

namespace libkerat {
    namespace message {

        typedef libkerat::message::skeleton::skeleton_graph skeleton_graph;

        const char * skeleton::PATH_2D = "/tuio2/skg";
        const char * skeleton::PATH_3D = "/tuio2/s3d";

        skeleton::skeleton(){ ; }

        skeleton::skeleton(const skeleton& original)
            :contact_session(original), message_output_mode(original),
            m_skeleton(original.m_skeleton)
        { ; }

        skeleton::skeleton(
            const session_id_t session_id,
            const skeleton_graph & skeleton,
            const helpers::message_output_mode::output_mode_t mode
        ) throw (libkerat::exception::invalid_graph_topology)
            :contact_session(session_id), message_output_mode(mode)
        { set_skeleton(skeleton); }

        // operators == != =
        skeleton & skeleton::operator=(const skeleton& original){
            contact_session::operator=(original);
            m_skeleton = original.m_skeleton;
            return *this;
        }
        bool skeleton::operator==(const skeleton& second) const {
            return (
                contact_session::operator==(second)
                && message_output_mode::operator==(second)
                && (m_skeleton == second.m_skeleton)
            );
        }

        skeleton_graph skeleton::set_skeleton(const skeleton_graph & skeleton)
            throw (libkerat::exception::invalid_graph_topology)
        {
            libkerat::message::skeleton::skeleton_graph retval = m_skeleton;

            bool is_empty = skeleton.empty();

            if (!is_empty){
                if (!graph_trunk_tree_is_valid(skeleton)){
                    throw exception::graph_error("Invalid graph topology for this type of message!");
                }
            }

            m_skeleton = skeleton;

            return retval;
        }

        bool skeleton::imprint_lo_messages(lo_bundle target) const {
            if (target == NULL){ return false; }

            if (libkerat::internals::testbit(get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_2D)){
                lo_message msg = lo_message_new();
                lo_message_add_int32(msg, get_session_id());

                skeleton_node_lo_writer<libkerat::helpers::point_2d> node_writer(msg);
                skeleton_rollback_lo_writer rollback_writer(msg);
                skeleton_edge_empty_writer edge_writer;
                graph_topology_trunk_tree_walk(m_skeleton, node_writer, edge_writer, rollback_writer);

                lo_bundle_add_message(target, PATH_2D, msg);
            }
            if (libkerat::internals::testbit(get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_3D)){
                lo_message msg = lo_message_new();
                lo_message_add_int32(msg, get_session_id());

                skeleton_node_lo_writer<libkerat::helpers::point_3d> node_writer(msg);
                skeleton_rollback_lo_writer rollback_writer(msg);
                skeleton_edge_empty_writer edge_writer;
                graph_topology_trunk_tree_walk(m_skeleton, node_writer, edge_writer, rollback_writer);

                lo_bundle_add_message(target, PATH_3D, msg);
            }
            return true;
        }

        kerat_message * skeleton::clone() const { return new skeleton(*this); }

        void skeleton::print(std::ostream & output) const { output << *this; }

        void skeleton::move_x(coord_t factor){
            skeleton_node_mover_x mover(factor);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, mover, edge_writer, rollback_writer);
        }
        void skeleton::move_y(coord_t factor){
            skeleton_node_mover_y mover(factor);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, mover, edge_writer, rollback_writer);
        }
        void skeleton::move_z(coord_t factor){
            skeleton_node_mover_z mover(factor);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, mover, edge_writer, rollback_writer);
        }

        void skeleton::scale_x(float factor, const helpers::point_2d & center){
            skeleton_node_scaler_x scaler(factor, center);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, scaler, edge_writer, rollback_writer);
        }
        void skeleton::scale_y(float factor, const helpers::point_2d & center){
            skeleton_node_scaler_y scaler(factor, center);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, scaler, edge_writer, rollback_writer);
        }
        void skeleton::scale_z(float factor, const helpers::point_3d & center){
            skeleton_node_scaler_z scaler(factor, center);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, scaler, edge_writer, rollback_writer);
        }

        void skeleton::rotate_by(angle_t angle, const helpers::point_2d & center){
            skeleton_node_rotator rotor(angle, center, &libkerat::rotate_around_center_yaw);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, rotor, edge_writer, rollback_writer);
        }

        void skeleton::rotate_pitch(angle_t angle, const helpers::point_3d & center){
            skeleton_node_rotator rotor(angle, center, libkerat::rotate_around_center_pitch);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, rotor, edge_writer, rollback_writer);
        }

        void skeleton::rotate_roll(angle_t angle, const helpers::point_3d & center){
            skeleton_node_rotator rotor(angle, center, libkerat::rotate_around_center_roll);
            skeleton_edge_empty_writer edge_writer;
            skeleton_rollback_empty_writer rollback_writer;
            graph_topology_trunk_tree_walk(m_skeleton, rotor, edge_writer, rollback_writer);
        }
    }

    namespace internals {
        namespace parsers {
            /**
            * \brief TUIO 2.0 skeleton geometry 2D (/tuio2/skg) parser
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
            bool parse_skg_2d(
                std::vector<libkerat::kerat_message *> & results,
                const char * path,
                const char * types,
                lo_arg ** argv,
                int argc,
                void * user_data __attribute__((unused))
            ){
                if (strcmp(path, libkerat::message::skeleton::PATH_2D) != 0){ return false; }
                if (types[0] != LO_INT32){ return false; }

                libkerat::session_id_t sid = argv[0]->i;
                typedef libkerat::message::skeleton::skeleton_graph skeleton_graph;
                typedef std::stack<skeleton_graph::node_iterator> node_stack;

                skeleton_graph skeleton;
                node_stack nodes;

                int i = 1;
                while (i < argc){
                    if (types[i] == LO_INT32){
                        size_t rollbacks = argv[i]->i;

                        assert(rollbacks < nodes.size());
                        for (size_t j = 0; j < rollbacks; j++){ nodes.pop(); }

                        ++i;
                    } else if ((types[i] == LO_FLOAT) && (types[i+1] == LO_FLOAT)){
                        libkerat::helpers::point_2d pt(argv[i]->f, argv[i+1]->f);
                        skeleton_graph::node_iterator current_node = skeleton.create_node(pt);

                        if (!nodes.empty()){
                            skeleton.create_edge(*(nodes.top()), *current_node);
                        }

                        nodes.push(current_node);
                        i += 2;
                    } else {
                        return false;
                    }
                }

                libkerat::message::skeleton * msg_skg = new libkerat::message::skeleton(sid, skeleton);
                msg_skg->set_message_output_mode(helpers::message_output_mode::OUTPUT_MODE_2D);
                results.push_back(msg_skg);
                return true;
            }

            /**
            * \brief TUIO 2.0 skeleton geometry 3D (/tuio3/s3d) parser
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
            bool parse_skg_3d(
                std::vector<libkerat::kerat_message *> & results,
                const char * path,
                const char * types,
                lo_arg ** argv,
                int argc,
                void * user_data __attribute__((unused))
            ){
                if (strcmp(path, libkerat::message::skeleton::PATH_3D) != 0){ return false; }
                if (types[0] != LO_INT32){ return false; }

                libkerat::session_id_t sid = argv[0]->i;
                typedef libkerat::message::skeleton::skeleton_graph skeleton_graph;
                typedef std::stack<skeleton_graph::node_iterator> node_stack;

                skeleton_graph skeleton;
                node_stack nodes;

                int i = 1;
                while (i < argc){
                    if (types[i] == LO_INT32){
                        size_t rollbacks = argv[i]->i;

                        assert(rollbacks < nodes.size());
                        for (size_t j = 0; j < rollbacks; j++){ nodes.pop(); }

                        ++i;
                    } else if ((types[i] == LO_FLOAT) && (types[i+1] == LO_FLOAT) && (types[i+2] == LO_FLOAT)){
                        libkerat::helpers::point_3d pt(argv[i]->f, argv[i+1]->f, argv[i+2]->f);
                        skeleton_graph::node_iterator current_node = skeleton.create_node(pt);

                        if (!nodes.empty()){
                            skeleton.create_edge(*(nodes.top()), *current_node);
                        }

                        nodes.push(current_node);
                        i += 3;
                    } else {
                        return false;
                    }
                }

                libkerat::message::skeleton * msg_s3d = new libkerat::message::skeleton(sid, skeleton);
                msg_s3d->set_message_output_mode(helpers::message_output_mode::OUTPUT_MODE_3D);
                results.push_back(msg_s3d);
                return true;
            }
        } // ns parsers
    } // ns internals
} // ns libkerat

/**
 * \ingroup global
 * \brief Pretty-print the \ref libkerat::message::skeleton "skeleton geometry" message to given output stream
 * \note Format for 2D variant: <tt>
 * {OSC path} {session id}[ {\ref libkerat::helpers::point_2d "point"}]+[ {node}[ {\ref libkerat::helpers::point_2d "point"}]+]*
 * </tt>
 * \note Format for 3D variant: <tt>
 * {OSC path} {session id}[ {\ref libkerat::helpers::point_3d "point"}]+[ {node}[ {\ref libkerat::helpers::point_3d "point"}]+]*
 * </tt>
 * \param output - output stream to write to
 * \param msg_skg - skeleton geometry message to print
 * \return modified output stream
 */
std::ostream & operator<<(std::ostream & output, const libkerat::message::skeleton & msg_skg){


    if (libkerat::internals::testbit(msg_skg.get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_2D)){
        output << libkerat::message::skeleton::PATH_2D
            << " " << msg_skg.get_session_id();

        skeleton_node_ostream_writer<libkerat::helpers::point_2d> node_writer(output);
        skeleton_rollback_ostream_writer rollback_writer(output);
        skeleton_edge_empty_writer edge_writer;
        libkerat::graph_topology_trunk_tree_walk(msg_skg.get_skeleton(), node_writer, edge_writer, rollback_writer);
    }
    if (libkerat::internals::testbit(msg_skg.get_message_output_mode(), libkerat::helpers::message_output_mode::OUTPUT_MODE_3D)){
        output << libkerat::message::skeleton::PATH_3D
            << " " << msg_skg.get_session_id();

        skeleton_node_ostream_writer<libkerat::helpers::point_3d> node_writer(output);
        skeleton_rollback_ostream_writer rollback_writer(output);
        skeleton_edge_empty_writer edge_writer;
        libkerat::graph_topology_trunk_tree_walk(msg_skg.get_skeleton(), node_writer, edge_writer, rollback_writer);
    }

    return output;
}
