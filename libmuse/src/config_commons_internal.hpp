/**
 * \file      config_commons_internal.hpp
 * \brief     The configuration auxiliary files
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-08-07 01:17 UTC+2
 * \copyright BSD
 */


#ifndef MUSE_CONFIG_COMMONS_INTERNAL_HPP
#define MUSE_CONFIG_COMMONS_INTERNAL_HPP

#include <tinyxml.h>

static const char * config_key_text_value(const TiXmlElement * root, const char * key_name){
    const TiXmlElement * e_key = root->FirstChildElement(key_name);
    if (e_key != NULL){
        return e_key->GetText();
    }
    return NULL;
}

inline bool config_key_to_element(const TiXmlElement * root, const char * key_name, const TiXmlElement ** element){
    *element = root->FirstChildElement(key_name);
    return (*element != NULL);
}

inline bool config_key_to_double(const TiXmlElement * root, const char * key_name, double & value){
    const char * tmp_value = config_key_text_value(root, key_name);
    if (tmp_value == NULL){ return false; }
    value = strtod(tmp_value, NULL);
    return true;
}

inline bool config_key_to_float(const TiXmlElement * root, const char * key_name, float & value){
    const char * tmp_value = config_key_text_value(root, key_name);
    if(tmp_value == NULL){ return false; }
    
    char * endptr = NULL;
    value = strtof(tmp_value, &endptr);
    if(endptr != (tmp_value + strlen(tmp_value))) { return false; }
    
    return true;
}

inline bool config_key_to_long(const TiXmlElement * root, const char * key_name, long & value){
    const char * tmp_value = config_key_text_value(root, key_name);
    if (tmp_value == NULL){ return false; }
    
    char * endptr = NULL;
    value = strtol(tmp_value, &endptr, 10);
    if(endptr != (tmp_value + strlen(tmp_value))) { return false; }
    
    return true;
}

inline bool config_attr_to_long(const TiXmlElement * root, const char * key_name, long & value){
    const char * tmp_value = root->Attribute(key_name);
    if (tmp_value == NULL){ return false; }
    if (!strlen(tmp_value)) { return false; }
    
    char * endptr = NULL;
    value = strtol(tmp_value, &endptr, 10);
    if(endptr != (tmp_value + strlen(tmp_value))) { return false; }
    
    return true;
}

inline bool config_key_to_string(const TiXmlElement * root, const char * key_name, std::string & value){
    const char * tmp_value = config_key_text_value(root, key_name);
    if (tmp_value == NULL){ return false; }
    value = tmp_value;
    return true;
}

inline bool config_attr_to_string(const TiXmlElement * root, const char * key_name, std::string & value){
    const char * tmp_value = root->Attribute(key_name);
    if (tmp_value == NULL){ return false; }
    value = tmp_value;
    return true;
}

inline bool config_key_to_bool(const TiXmlElement * root, const char * key_name, bool & value){
    const char * tmp_value = config_key_text_value(root, key_name);
    if (tmp_value == NULL){ value = false; return false; }

    // off
    if ((strcasecmp(tmp_value, "off") == 0)
        || (strcasecmp(tmp_value, "false") == 0)
        || (strcmp(tmp_value, "0") == 0)
    ){
        value = false;
    } else if ((strcasecmp(tmp_value, "on") == 0)
        || (strcasecmp(tmp_value, "true") == 0)
        || (strcmp(tmp_value, "1") == 0)
        || (strcmp(tmp_value, key_name) == 0)
    ){ 
        value = true;
    } else {
        return false;
    }

    return true;
}

inline bool config_attr_to_bool(const TiXmlElement * root, const char * key_name, bool & value){
    const char * tmp_value = root->Attribute(key_name);
    if (tmp_value == NULL){ value = false; return false; }

    // off
    if ((strcasecmp(tmp_value, "off") == 0)
        || (strcasecmp(tmp_value, "false") == 0)
        || (strcmp(tmp_value, "0") == 0)
    ){
        value = false;
    } else if ((strcasecmp(tmp_value, "on") == 0)
        || (strcasecmp(tmp_value, "true") == 0)
        || (strcmp(tmp_value, "1") == 0)
        || (strcmp(tmp_value, key_name) == 0)
    ){ 
        value = true;
    } else {
        return false;
    }

    return true;
}

#endif // MUSE_CONFIG_COMMONS_INTERNAL_HPP
