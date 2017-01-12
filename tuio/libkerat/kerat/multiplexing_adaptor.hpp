/**
 * \file      multiplexing_adaptor.hpp
 * \brief     Provides the multiplexing TUIO 2.0 adaptor.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-12 16:31 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_MULTIPLEXING_ADAPTOR_HPP
#define KERAT_MULTIPLEXING_ADAPTOR_HPP

#include <lo/lo.h>
#include <kerat/typedefs.hpp>
#include <kerat/adaptor.hpp>
#include <kerat/session_manager.hpp>

namespace libkerat {

    namespace adaptors {

        //! \brief Adaptor that provides the multiplexing capability for \ref libkerat::client TUIO clients
        class multiplexing_adaptor: public adaptor, private internals::session_manager {
        public:

            //! \brief Create a new multiplexing adaptor
            multiplexing_adaptor(){ ; }

            virtual ~multiplexing_adaptor(){ ; }

            void notify(const client * notifier);

            void purge();

            bundle_stack get_stack() const { return m_processed_frames; }

            /**
             * \brief Map session ids of the received bundle to this multiplexor scope
             *
             * Each session id from to_process bundle is mapped to new session id space
             * that is common for all bundle sources which are processed through this adaptor.
             *
             * \param to_process - received frame handle
             * \param output_frame - mapped frame handle
             * \return 0 if all contained session ids were remapped, negative number if an error has occured
             */
            int process_bundle(const bundle_handle & to_process, bundle_handle & output_frame);

        private:

            //! \brief The mapping type that holds actual id to id mapping
            typedef std::map<libkerat::session_id_t, libkerat::session_id_t> internal_session_id_map;

            //! \brief The type uniquely identyfying the source.
            typedef struct source_key_type {
                bool operator<(const source_key_type & second) const ;

                //! \brief Source IPv4 address of the bundle
                libkerat::addr_ipv4_t addr;
                //! \brief Source tracker instance of the bundle
                libkerat::instance_id_t instance;
                //! \brief Source tracker name
                std::string application;
            } source_key_type;

            //! \brief The mapping type that holds the id mapping for source
            typedef std::map<source_key_type, internal_session_id_map> session_id_map;

            //! \brief The type that holds alive associations
            typedef std::map<source_key_type, libkerat::message::alive_associations::associated_ids> associations_map;

        protected:
            /**
             * \brief Get mapped id for given source and id
             *
             * If the correspondign (source+id)->id mapping already exists, it is returned.
             * If the mapping does not exist, a new id is allocated.
             *
             * \param source - source of the tuio bundle containing the session id sid
             * \param sid    - session id to map from
             * \return session id that corresponds to the unique combination of source and sid
             */
            libkerat::session_id_t get_mapped_id(const source_key_type & source, const libkerat::session_id_t sid);

            /**
             * \brief Removes the dead session id's
             *
             * Removes the dead session id's received from source from the session
             * mapping and possibly creates new mappings if the corresponding mapping did not exist yet
             *
             * \param source - source from which the id set came
             * \param update - the update id set
             */
            void update_alives(const source_key_type & source, const message::alive::alive_ids & update);

            /**
             * \brief Removes the dead associations and adds new
             *
             * Removes the dead associations received from source from the associations
             * mapping and possibly creates new mappings if the corresponding mapping did not exist yet
             *
             * \param source - source from which the id set came
             * \param update - the update id set
             */
            void update_associations(const source_key_type & source, const message::alive_associations::associated_ids & update);
            /**
             * \brief remap session id's in association-type messages
             * \tparam T - association class message to run remapping process on
             * \param[in,out] message - message to run remapping on
             * \param source - source of the bundle conatining the message remapped
             */
            template <class T> void rempap_associated_ids(T & message, const source_key_type & source);

            /**
             * \brief remap session id's in link topology messages
             * \tparam T - link topology class message to run remapping process on
             * \param[in,out] message - message to run remapping on
             * \param source - source of the bundle conatining the message remapped
             */
            template <class T> void rempap_links(T & message, const source_key_type & source);

            //! \brief generate new alive message content
            libkerat::message::alive::alive_ids get_alives() const;

            //! \brief generate new alive association message content
            libkerat::message::alive_associations::associated_ids get_associations() const;

        private:
            session_id_map m_mapping;
            associations_map m_associations;
            bundle_stack m_processed_frames;

        };
    }

} // ns libkerat


#endif // KERAT_MULTIPLEXING_ADAPTOR_HPP
