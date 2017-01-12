/**
 * \file      bundle.hpp
 * \brief     Provides handle and stack classes for the TUIO event bundle delivery
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-26 13:00 UTC+2
 * \copyright BSD
 */

#ifndef KERAT_BUNDLE_HPP
#define KERAT_BUNDLE_HPP

#include <kerat/typedefs.hpp>
#include <kerat/tuio_messages.hpp>
#include <iterator>
#include <list>
#include <map>
#include <deque>
#include <iostream>

namespace libkerat {

    class bundle_handle;
    class client;
    
    namespace internals {
        class bundle_manipulator;
    }

    /**
     * \brief Handle for TUIO 2.0 event bundle
     * 
     * The received data is considered to be of WORM character, so internals use
     * reference counters to hold the data.
     */
    class bundle_handle {
    private:

        //! \brief Pre-destructor cleanup, manipulates the internal reference counter
        void try_clear();

    protected:

        //! \brief Internal representation of the event bundle
        struct reference_bundle_handle {
            reference_bundle_handle();
            ~reference_bundle_handle();
            
            void clear();

            typedef std::list<kerat_message *> message_list;

            friend class internals::bundle_manipulator;
            
            //! \brief References to bundle_handle instances using this storage
            uint32_t reference_count;

            //! \brief The very stored messages
            message_list stored_messages;
        };

        /**
         * \brief Creates a deep copy of this frame handle with storage of it's own
         * \return Newly allocated copy
         */
        bundle_handle * clone() const;

        /**
         * \brief Flushes the message stack. Adding the frame message does not call this automaticaly
         * \note Affects all bundle_handles that access the same data!
         */
        void clear();
        
    public:
        //! \brief Iterator to stored messages
        typedef reference_bundle_handle::message_list::const_iterator const_iterator;

        //! \brief Create a new, empty bundle handle
        bundle_handle();

        //! \brief Create a new bundle handle that accesses the same data as given bundle handle
        bundle_handle(const bundle_handle & second);

        /**
         * \brief Destroy this handle.
         * \note If this is the last bundle handle accessing the data, the data are
         * destroyed as well.
         */
        ~bundle_handle();

        //! \brief Accesses the same data as given bundle handle
        bundle_handle & operator=(const bundle_handle & second);

        /**
         * \brief Gets the bundle opening TUIO 2.0 frame message
         * \return Valid frame message or NULL if not set - possible due to mallformed bundle
         */
        inline const message::frame * get_frame() const { return get_message_of_type<message::frame>(0); }

        /**
         * \brief Gets the closing TUIO 2.0 alive message
         * \return Valid alive message or NULL if not set - possible due to mallformed bundle
         */
        inline const message::alive * get_alive() const { return get_message_of_type<message::alive>(0); }

        /**
         * \brief Gets the indexed message of given type
         * \tparam type - type of messages to filter out
         * \param index - the position of message to extract (relative to all messages of this type)
         * \return NULL if pointer with given index does not exist
         */
        template <typename T>
        const T * get_message_of_type(uint32_t index = 0) const {
        
            const T * retval = NULL;

            // slow, naive algorithm
            uint32_t cur = 0;
            typedef reference_bundle_handle::message_list::const_iterator const_iterator;

            for (const_iterator i = m_messages_held->stored_messages.begin(); (retval == NULL) && (i != m_messages_held->stored_messages.end()); i++){
                const T * tmp = dynamic_cast<const T *>(*i);
                if (tmp != NULL){
                    if (index == cur){
                        retval = dynamic_cast<const T *>(*i);;
                    }
                    ++cur;
                }
            }

            return retval;
            
        }

        /**
         * \brief Get end iterator for stored messages
         * \return const past-the-end iterator
         */
        inline const_iterator end() const { return m_messages_held->stored_messages.end(); }

        /**
         * \brief Get iterator to first of stored messages
         * \return const iterator to first of stored messages if such exists,
         * \ref end otherwise
         */
        inline const_iterator begin() const { return m_messages_held->stored_messages.begin(); }

        /**
         * \brief Checks whether this bundle handle holds any messages
         * \return true if this bundle is not empty
         */
        inline bool empty(){ return m_messages_held->stored_messages.empty(); }

    protected:

        friend class internals::bundle_manipulator;

        //! \brief the very TUIO messages bundle that this handle accesses
        reference_bundle_handle * m_messages_held;

    };

    /**
     * \brief Class used to access all TUIO event bundles that were loaded during
     * single \ref libkerat::client::load call
     */
    class bundle_stack {
    public:

        //! \brief Index of the oldest event bundle stored in this stack
        static const size_t INDEX_OLDEST = 0;

        //! \brief Deep copy the given stack
        bundle_stack(const bundle_stack & other);

        //! \brief Create a new, empty bundle stack
        bundle_stack();
        
        ~bundle_stack();
        
        bundle_stack & operator=(const bundle_stack & other);

        /**
         * \brief Get the count of events in stack
         * \return count of events in this stack
         */
        size_t get_length();

        /**
         * Get the event bundle for given index (the index can be either \ref INDEX_OLDEST
         * or any non-negative number lower than value returned by \ref get_length
         * \note This method erases the indexed event bundle and all older ones from this event stack
         * \return const bundle_handle for given event
         * \throws std::out_of_range if given index is too great or out of range
         */
        const bundle_handle get_update(size_t index = INDEX_OLDEST) throw (std::out_of_range);

    private:
        typedef std::deque<bundle_handle *> aux_stack;
        
        friend class internals::bundle_manipulator;

        //! \brief Erases all bundle handles this stack holds
        void clear();

        aux_stack m_stack;

    };

    namespace internals {

        /**
         * \brief Only this class is allowed to manipulate
         * \ref bundle_handle and \ref bundle_stack in read-write way
         * \relates bundle_handle
         * \relates bundle_stack
         *
         * If you want to access the TUIO event bundles in read-write way, you must
         * inherit from this class. It is meant for use in clients only as their
         * essential internal component. Methods below exist due to frendship limitations.
         */
        class bundle_manipulator {
        protected:

            typedef bundle_stack::aux_stack aux_bundle_stack;

            //! \brief Initialize the manipulator
            bundle_manipulator(){ ; }
            
            //! \brief Calls \ref bundle_stack::clear
            static void bm_stack_clear(bundle_stack & stack);
            
            //! \brief Appends given pointer to bundle handle to the bundle stack
            static void bm_stack_append(bundle_stack & stack, bundle_handle * appendix);

            //! \brief RW iterator type to manipulate the bundle
            typedef bundle_handle::reference_bundle_handle::message_list::iterator handle_iterator;
            
            //! \brief Calls \ref bundle_handle::clone
            static bundle_handle * bm_handle_clone(const bundle_handle & handle);
            
            //! \brief Creates a copy of source handle's messages to destination handles's
            static void bm_handle_copy(const bundle_handle & source, bundle_handle & destination);

            //! \brief Calls \ref bundle_handle::clear
            static void bm_handle_clear(bundle_handle & handle);
            
            //! \brief Inserts message into handle on given position
            static bool bm_handle_insert(bundle_handle & handle, handle_iterator where, libkerat::kerat_message * message);

            //! \brief Erases the message from handle on given position
            static bool bm_handle_erase(bundle_handle & handle, handle_iterator where);

            //! \brief Gets the begin rw iterator for handle
            static handle_iterator bm_handle_begin(bundle_handle & handle);

            //! \brief Gets the end rw iterator for handle
            static handle_iterator bm_handle_end(bundle_handle & handle);
        
        };
    }
}

#endif // KERAT_BUNDLE_HPP
