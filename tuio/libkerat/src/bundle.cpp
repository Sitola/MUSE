/**
 * \file      bundle.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-10-04 10:41 UTC+2
 * \copyright BSD
 */

#include <kerat/typedefs.hpp>
#include <kerat/bundle.hpp>
#include <deque>
#include <cstdio>
#include <sstream>

namespace libkerat {

    bundle_handle::reference_bundle_handle::reference_bundle_handle()
        :reference_count(1)
    {
        ;
    }

    bundle_handle::reference_bundle_handle::~reference_bundle_handle(){
        clear();
    }

    void bundle_handle::reference_bundle_handle::clear(){

        for (message_list::iterator i = stored_messages.begin(); i != stored_messages.end(); i++){
            delete *i;
        }

        stored_messages.clear();
    }


    bundle_handle::bundle_handle()
        :m_messages_held(NULL)
    {
        m_messages_held = new reference_bundle_handle;
    }

    bundle_handle::bundle_handle(const bundle_handle& second){
        m_messages_held = second.m_messages_held;
        ++(m_messages_held->reference_count);
    }

    bundle_handle::~bundle_handle(){
        try_clear();
    }

    bundle_handle & bundle_handle::operator =(const bundle_handle& second){
        if (this == &second){ return *this; }

        try_clear();

        m_messages_held = second.m_messages_held;
        ++(m_messages_held->reference_count);

        return *this;
    }

    void bundle_handle::try_clear(){
        if (m_messages_held != NULL){
            --(m_messages_held->reference_count);
            if (m_messages_held->reference_count == 0){
                delete m_messages_held;
                m_messages_held = NULL;
            }
        }
    }

    void bundle_handle::clear(){
        //while (m_messages_held->stored_messages.begin() != m_messages_held->stored_messages.end()){
        //    delete m_messages_held->stored_messages.front();
        //    m_messages_held->stored_messages.erase(m_messages_held->stored_messages.begin());
        //}
        m_messages_held->clear();
    }

    bundle_handle * bundle_handle::clone() const {
        bundle_handle * tmp = new bundle_handle;

        typedef bundle_handle::reference_bundle_handle::message_list::const_iterator iterator;
        for (iterator i = m_messages_held->stored_messages.begin(); i != m_messages_held->stored_messages.end(); i++){
            tmp->m_messages_held->stored_messages.push_back((*i)->clone());
        }

        return tmp;
    }

    bundle_stack::bundle_stack(const bundle_stack& other){
        (*this) = other;
    }

    bundle_stack & bundle_stack::operator=(const bundle_stack& other){
        clear();

        for (aux_stack::const_iterator i = other.m_stack.begin(); i != other.m_stack.end(); i++){
            bundle_handle * handle = new bundle_handle(*(*i));
            m_stack.push_back(handle);
        }

        return *this;
    }

    void bundle_stack::clear(){
        while (m_stack.begin() != m_stack.end()){
            bundle_handle * handle = *m_stack.begin();
            delete handle;
            handle = NULL;
            m_stack.erase(m_stack.begin());
        }
    }

    bundle_stack::bundle_stack(){ ; }
    bundle_stack::~bundle_stack(){
        clear();
    }


    const bundle_handle bundle_stack::get_update(size_t index) throw (std::out_of_range) {
        if (!(index < m_stack.size())){
            std::stringstream sx;
            sx << "Bundle with index "
                << index << " does not exist. There are only "
                << get_length() << " bundles in this stack!";
            std::string msg;
            getline(sx, msg);
            throw std::out_of_range(msg);
        }

        // select bundle
        aux_stack::iterator selected_bundle = m_stack.begin() + index;
        bundle_handle retval = *(*selected_bundle);

        // erase all previous bundles
        ++selected_bundle;
        for (aux_stack::iterator db = m_stack.begin(); db != selected_bundle; db++){
            delete *db;
            *db = NULL;
        }
        m_stack.erase(m_stack.begin(), selected_bundle);

        return retval;
    }

    size_t bundle_stack::get_length(){ return m_stack.size(); }


    namespace internals {
        bool bundle_manipulator::bm_handle_insert(bundle_handle & handle, bundle_manipulator::handle_iterator where, libkerat::kerat_message * message){
            handle.m_messages_held->stored_messages.insert(where, message);
            return true;
        }

        bool bundle_manipulator::bm_handle_erase(bundle_handle & handle, bundle_manipulator::handle_iterator where){
            handle.m_messages_held->stored_messages.erase(where);
            return true;
        }

        bundle_manipulator::handle_iterator bundle_manipulator::bm_handle_begin(bundle_handle & handle){
            return handle.m_messages_held->stored_messages.begin();
        }

        bundle_manipulator::handle_iterator bundle_manipulator::bm_handle_end(bundle_handle & handle){
            return handle.m_messages_held->stored_messages.end();
        }
        
        bundle_handle * bundle_manipulator::bm_handle_clone(const bundle_handle & handle){
            return handle.clone();
        }

        void bundle_manipulator::bm_handle_copy(const bundle_handle & source, bundle_handle & destination){
            if (source.m_messages_held == destination.m_messages_held){ return; }
            
            destination.clear();
            for (bundle_handle::const_iterator i = source.begin(); i != source.end(); ++i){
                destination.m_messages_held->stored_messages.push_back((*i)->clone());
            }
        }

        void bundle_manipulator::bm_handle_clear(bundle_handle & handle){
            handle.clear();
        }
        
        void bundle_manipulator::bm_stack_clear(bundle_stack & stack){ 
            stack.clear(); 
        }

        void bundle_manipulator::bm_stack_append(bundle_stack & stack, bundle_handle * appendix){ 
            stack.m_stack.push_back(appendix); 
        }
        
    }
}
