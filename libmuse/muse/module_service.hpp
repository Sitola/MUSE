#ifndef MUSE_MODULE_SERVICE
#define MUSE_MODULE_SERVICE
#include <kerat/kerat.hpp>
#include <tinyxml.h>
#include <vector>
#include <map>

namespace muse {

    typedef libkerat::adaptor muse_module;
    
    libkerat::internals::convertor_list get_muse_convertors();

    class module_container {
    public:
        virtual ~module_container(){ ; }
        
        virtual int create_module_instance(muse_module ** module, const TiXmlElement * module_config) const = 0;
    };

    class module_service {
    public:
        virtual ~module_service();
        
        typedef std::vector<std::pair<muse_module *, TiXmlElement *> > module_chain;
        
        static module_service * get_instance();

        int create_module_instance(muse_module ** module, const std::string & module_path, const TiXmlElement * module_config);
        muse_module * create_module_instance(const std::string & module_path, const TiXmlElement * module_config);

        bool has_module(const std::string & module_path);

        int register_module_container(const std::string & module_path, module_container * module_templ);
        module_container * unregister_module_container(const std::string & module_path);
        
        int create_module_chain(const TiXmlElement * chain_root, module_chain & resulting_chain);
        void free_module_chain(module_chain & chain);

    private:
        typedef std::map<std::string, module_container *> module_map;

        module_service();

        module_map m_registered_modules;
        static module_service * s_ms_instance;
    };
}

#endif // MUSE_MODULE_SERVICE
