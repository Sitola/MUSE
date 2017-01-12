/**
 * \file      multiplexing_adaptor.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-13 11:31 UTC+2
 * \copyright BSD
 */

#include <iostream>
#include <kerat/typedefs.hpp>
#include <kerat/multiplexing_adaptor.hpp>
#include <kerat/tuio_messages.hpp>

namespace libkerat {
    namespace adaptors {

        void multiplexing_adaptor::notify(const client * cl){
            purge();

            bundle_stack data = cl->get_stack();
            while (data.get_length() > 0){
                bundle_handle current_frame = data.get_update(bundle_stack::INDEX_OLDEST);
                bundle_handle * tmphx = new bundle_handle;
                process_bundle(current_frame, *tmphx);
                bm_stack_append(m_processed_frames, tmphx);
            }

            notify_listeners();
        }
        
        void multiplexing_adaptor::purge(){ bm_stack_clear(m_processed_frames); }

        session_id_t multiplexing_adaptor::get_mapped_id(const multiplexing_adaptor::source_key_type& source, const libkerat::session_id_t sid){

            session_id_t retval;

            // if the mapping map is not present yet, create it
            internal_session_id_map & srcmap = m_mapping[source];
            // attempt to find mapped id
            internal_session_id_map::const_iterator sval = srcmap.find(sid);

            // if found return it, create new else
            if (sval == srcmap.end()){
                retval = get_next_session_id();
                srcmap[sid] = retval;
            } else {
                retval = sval->second;
            }

            return retval;

        }

        void multiplexing_adaptor::update_alives(const source_key_type& source, const message::alive::alive_ids & update_full){
            typedef message::alive::alive_ids alive_ids;

            internal_session_id_map & srcmap = m_mapping[source];

            // remove mappings for already-present nodes
            // add mappings for yet unmapped id's
            const internal_session_id_map srcmap_full(srcmap);
            message::alive::alive_ids update(update_full);

            for (internal_session_id_map::const_iterator current = srcmap_full.begin(); current != srcmap_full.end(); current++){
                alive_ids::const_iterator cs = update_full.find(current->first);
                if (cs == update_full.end()){
                    srcmap.erase(current->first);
//                } else {
//                    update.erase(*cs);
                }
            }

            for (alive_ids::iterator current = update_full.begin(); current != update_full.end(); current++){
                get_mapped_id(source, *current);
            }
        }

        void multiplexing_adaptor::update_associations(const source_key_type& source, const message::alive_associations::associated_ids& update){
            m_associations[source] = update;
        }

        int multiplexing_adaptor::process_bundle(const bundle_handle& to_process, bundle_handle& output_frame){

            // make sure we won't have to deal with the self-asignment problem
            bool copy = false;
            const bundle_handle * input = &to_process;
            if (&to_process == &output_frame){
                copy = true;
                input = bm_handle_clone(output_frame);
            }

            bm_handle_clear(output_frame);

            // from now on, do the useful stuff
            source_key_type source;
            bool out_of_bundle = true;

            // process intermediate messages
            typedef bundle_handle::const_iterator iterator;
            for (iterator i = input->begin(); i != input->end(); i++){

                kerat_message * tmp = (*i)->clone();

                { // frame
                    message::frame * msg_frame = dynamic_cast<message::frame*>(tmp);
                    if (msg_frame != NULL){
                        source.addr = msg_frame->get_address();
                        source.instance = msg_frame->get_instance();
                        source.application = msg_frame->get_app_name();
                        out_of_bundle = false;
                        goto msg_push;
                    }
                }

                { // alive
                    message::alive * msg_alive = dynamic_cast<message::alive*>(tmp);
                    if (msg_alive != NULL){
                        update_alives(source, msg_alive->get_alives());
                        msg_alive->set_alives(get_alives());
                        out_of_bundle = true;
                        goto msg_push;
                    }
                }

                { // alive associations
                    message::alive_associations * msg_ala = dynamic_cast<message::alive_associations*>(tmp);
                    if (msg_ala != NULL){
                        update_associations(source, msg_ala->get_associations());
                        msg_ala->set_associations(get_associations());
                        goto msg_push;
                    }
                }

                { // conatainer associations
                    message::container_association * msg_coa = dynamic_cast<message::container_association*>(tmp);
                    if (msg_coa != NULL){
                        rempap_associated_ids(*msg_coa, source);
                        goto sid_remap;
                    }
                }

                // do not use helper here, someone might utilize the link_topology
                // helper as well but in incompatible way!
                { // link association
                    message::link_association * msg_lia = dynamic_cast<message::link_association*>(tmp);
                    if (msg_lia != NULL){
                        rempap_links(*msg_lia, source);
                        goto msg_push;
                    }
                }
                { // linked list association
                    message::linked_list_association * msg_lla = dynamic_cast<message::linked_list_association*>(tmp);
                    if (msg_lla != NULL){
                        rempap_links(*msg_lla, source);
                        goto msg_push;
                    }
                }
                { // linked tree association
                    message::linked_tree_association * msg_lta = dynamic_cast<message::linked_tree_association*>(tmp);
                    if (msg_lta != NULL){
                        rempap_links(*msg_lta, source);
                        goto msg_push;
                    }
                }

                // the message was not recognized so far, let's detect whether it even has session id
                // allow jump to for coa-like messages
            sid_remap:
                {
                    helpers::contact_session * msg_session = dynamic_cast<helpers::contact_session*>(tmp);
                    if (msg_session != NULL) {
                        msg_session->set_session_id(get_mapped_id(source, msg_session->get_session_id()));
                    }
                }
                // all messages shall be pushed
            msg_push:
                bm_handle_insert(output_frame, bm_handle_end(output_frame), tmp);
                tmp = NULL;
            }

            // the input bundle handle was duplicated, erase it
            if (copy){ delete input; }

            // check for errors during transformation
            if (out_of_bundle){
                return 1;
            } else {
                return 0;
            }
        }

        libkerat::message::alive::alive_ids multiplexing_adaptor::get_alives() const {
            libkerat::message::alive::alive_ids alives;

            for (session_id_map::const_iterator sr = m_mapping.begin(); sr != m_mapping.end(); sr++){
                for (internal_session_id_map::const_iterator sid = sr->second.begin(); sid != sr->second.end(); sid++){
                    alives.insert(sid->second);
                }
            }

            return alives;
        }
        libkerat::message::alive_associations::associated_ids multiplexing_adaptor::get_associations() const {
            typedef libkerat::message::alive_associations::associated_ids associated_ids;
            associated_ids associations;

            for (associations_map::const_iterator sr = m_associations.begin(); sr != m_associations.end(); sr++){
                associations.insert(sr->second.begin(), sr->second.end());
            }

            return associations;
        }

        template <class T> void multiplexing_adaptor::rempap_associated_ids(T& message, const source_key_type& source){
            typename T::associated_ids ids_new;
            const typename T::associated_ids & originals = message.get_associations();

            for (typename T::associated_ids::const_iterator i = originals.begin(); i != originals.end(); i++){
                ids_new.insert(get_mapped_id(source, *i));
            }

            message.set_associations(ids_new);
        }

        template <class T> void multiplexing_adaptor::rempap_links(T& message, const source_key_type& source){
            typedef libkerat::helpers::link_topology link_topology;
            link_topology::internal_link_graph link_graph = message.get_link_graph();
            for (typename link_topology::internal_link_graph::node_iterator node = link_graph.nodes_begin(); node != link_graph.nodes_end(); node++){
                node->set_value(get_mapped_id(source, node->get_value()));
            }
            message.set_link_graph(link_graph);
        }

        bool multiplexing_adaptor::source_key_type::operator<(const multiplexing_adaptor::source_key_type & second) const {
            if (addr < second.addr){
                return true;
            } else if (addr == second.addr) {
                if (instance < second.instance){
                    return true;
                } else if (instance == second.instance) {
                    return (application.compare(second.application) < 0);
                }
            }

            return false;
        }


    } // ns adaptors
} // ns libkerat
