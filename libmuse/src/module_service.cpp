/**
 * \file      module_service.cpp
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-07-27 16:11 UTC+2
 * \copyright BSD
 */

#include <muse/module_service.hpp>

namespace muse {

    module_service::module_service(){ ; }
    module_service::~module_service(){ ; }
    
	module_service * module_service::get_instance() {
		if (s_ms_instance == NULL){
			s_ms_instance = new module_service;
		}
		return s_ms_instance;
	}

	int module_service::create_module_instance(muse_module ** module, const std::string & module_path, const TiXmlElement * module_config){
		*module = NULL;

		typedef module_service::module_map::const_iterator const_iterator;
		const_iterator module_entry = m_registered_modules.find(module_path);
		if (module_entry == m_registered_modules.end()) { return -1; }

		int retval = module_entry->second->create_module_instance(module, module_config);
		return retval;
	}

	muse_module * module_service::create_module_instance(const std::string & module_path, const TiXmlElement * module_config){
		muse_module * module = NULL;

		int rv = create_module_instance(&module, module_path, module_config);
		// posix-like error detection, ensure safe return state
		if (rv != 0) { module = NULL; }

		return module;
	}

	bool module_service::has_module(const std::string & module_path){
		typedef module_map::const_iterator const_iterator;
		const_iterator module_entry = m_registered_modules.find(module_path);
		return !(module_entry == m_registered_modules.end());
	}

	int module_service::register_module_container(const std::string & module_path, module_container * module_templ){
		typedef module_map::const_iterator const_iterator;
		const_iterator module_entry = m_registered_modules.find(module_path);
		if (module_entry != m_registered_modules.end()) { return -1; }

		m_registered_modules.insert(module_map::value_type(module_path, module_templ));
		return 0;
	}
	module_container * module_service::unregister_module_container(const std::string & module_path){
		typedef module_map::iterator iterator;
		iterator module_entry = m_registered_modules.find(module_path);
		if (module_entry == m_registered_modules.end()) { return NULL; }

		module_container * module = module_entry->second;
        m_registered_modules.erase(module_entry);
        
        return module;
	}
    
    int module_service::create_module_chain(const TiXmlElement * chain_root, module_service::module_chain & resulting_chain){
        resulting_chain.clear();
        int retval = 0;
        
        const TiXmlElement * e_current_module = chain_root->FirstChildElement("module");
        while ((e_current_module != NULL) && (retval == 0)){
            const char * module_path = e_current_module->Attribute("path");
            const TiXmlElement * e_config = e_current_module->FirstChildElement("config");
            
            if (module_path == NULL){ retval = -1; continue; }
            muse_module * module_instance = NULL;
            
            retval = create_module_instance(&module_instance, module_path, e_config);
            if (module_instance == NULL) {
                if (retval == 0){
                    retval = -1;
                }
            }
            TiXmlElement * e_used_module = (TiXmlElement *)(e_current_module->Clone());
            resulting_chain.push_back(module_chain::value_type(module_instance, e_used_module));
            
            e_current_module = e_current_module->NextSiblingElement("module");
        }
        
        if ((retval == 0) && (!resulting_chain.empty())){
            module_chain::iterator previous_module = resulting_chain.begin();
            module_chain::iterator current_module = ++resulting_chain.begin();
            for ( ; current_module != resulting_chain.end(); ++current_module){
                previous_module->first->add_listener(current_module->first);
                previous_module = current_module;
            }
        }
        
        return retval;
    }
    void module_service::free_module_chain(module_service::module_chain& chain){
        for ( 
            module_chain::iterator current_module = chain.begin();
            current_module != chain.end(); 
            ++current_module
        ){
            delete current_module->first;
            current_module->first = NULL;
            delete current_module->second;
            current_module->second = NULL;
        }
        chain.clear();
    }
}

