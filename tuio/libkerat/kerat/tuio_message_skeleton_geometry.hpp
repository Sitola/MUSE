/**
 * \file      tuio_message_skeleton_geometry.hpp
 * \brief     TUIO 2.0 skeleton geometry family (/tuio2/s3d, /tuio2/skg)
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-04-25 16:16 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_TUIO_MESSAGE_SKELETON_GEOMETRY_HPP
#define KERAT_TUIO_MESSAGE_SKELETON_GEOMETRY_HPP

#include <string>
#include <kerat/typedefs.hpp>
#include <kerat/message.hpp>
#include <kerat/message_helpers.hpp>
#include <list>
#include <ostream>

namespace libkerat {
    namespace message {

        //! \brief TUIO 2.0 skeleton geometry family (/tuio2/s3d, /tuio2/skg)
        class skeleton: public libkerat::kerat_message,
            public libkerat::helpers::contact_session,
            public libkerat::helpers::message_output_mode,
            public libkerat::helpers::rotatable_cs_3d,
            public libkerat::helpers::movable_3d,
            public libkerat::helpers::scalable_3d
        {
        public:
            /**
             * \brief Skeleton graph type. Node values are
             * \ref libkerat::helpers::point_3d "points",
             * edges are of bool type (ignored)
             */
            typedef internals::graph<helpers::point_3d, bool> skeleton_graph;

            /**
             * \brief Creates new, empty TUIO 2.0 skeleton geometry
             *
             * All message components (and corresponding helpers) are initialized
             * through their default constructors. The message output mode is set
             * to \ref OUTPUT_MODE_BOTH.
             */
            skeleton();

            //! \brief Create a deep copy of this message
            skeleton(const skeleton & original);

            /**
             * \brief Creates new TUIO 2.0 skeleton geometry from given data
             *
             * \param session_id - session id of the contact
             * \param skeleton - skeleton of the contact blob, see
             * \ref set_skeleton for details and input requirements
             * \param mode - message output mode, defaults to
             * \ref libkerat::helpers::message_output_mode::OUTPUT_MODE_2D "OUTPUT_MODE_2D"
             * \throws libkerat::exception::invalid_graph_topology when the
             * skeleton graph does not fulfill the input requrements
             */
            skeleton(
                const session_id_t session_id,
                const skeleton_graph & skeleton,
                const message_output_mode::output_mode_t mode = message_output_mode::OUTPUT_MODE_2D
            ) throw (libkerat::exception::invalid_graph_topology);

            skeleton & operator=(const skeleton & second);
            bool operator == (const skeleton & second) const ;
            inline bool operator != (const skeleton & second) const { return !operator==(second); }

            /**
             * \brief Sets the skeleton graph
             * \param skeleton - the very skeleton graph
             * \throws libkerat::exception::invalid_graph_topology when the skeleton graph does
             * not fulfill the input requrements
             * \note The link graph must fulfill all of the following requirements:
             *   \li Single component oriented graph
             *   \li Tree-like topology (trunk-tree) - the graph begins with leaf (origin leaf)
             *   from which leads the oriented edge to the first node, from this node
             *   to next until a split-node is found (like a trunk of real tree) - from
             *   now on, the rest of this tree can be considered classical oriented tree.
             *   All nodes and leafs are reachable from the origin leaf.
             * \return previous setting
             */
            skeleton_graph set_skeleton(const skeleton_graph & skeleton) throw (libkerat::exception::invalid_graph_topology);

            /**
             * \brief Gets the skeleton graph
             * \return skeleton graph or empty graph if not set
             */
            inline const skeleton_graph & get_skeleton() const { return m_skeleton; }

            kerat_message * clone() const ;

            void print(std::ostream & output) const ;

            // move, scale a rotatable overrides
            void scale_x(float factor, const helpers::point_2d & center);
            void scale_y(float factor, const helpers::point_2d & center);
            void scale_z(float factor, const helpers::point_3d & center);
            void move_x(coord_t factor);
            void move_y(coord_t factor);
            void move_z(coord_t factor);
            void rotate_by(angle_t angle, const helpers::point_2d & center);
            void rotate_pitch(angle_t angle, const helpers::point_3d & center);
            void rotate_roll(angle_t angle, const helpers::point_3d & center);

            //! \brief OSC path for the 2D pointer
            static const char * PATH_2D;
            //! \brief OSC path for the 3D pointer
            static const char * PATH_3D;

        private:

            skeleton_graph m_skeleton;

            bool imprint_lo_messages(lo_bundle target) const;

        }; // cls skeleton

    } // ns message
} // ns libkerat

std::ostream & operator<<(std::ostream & output, const libkerat::message::skeleton &  msg_skg);

#endif // KERAT_TUIO_MESSAGE_SKELETON_GEOMETRY_HPP
