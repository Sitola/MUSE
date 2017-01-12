/**
 * \file      framework_init.cpp
 * \brief     Here are all the framework initialization routines defined
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-07-27 14:29 UTC+2
 * \copyright BSD
 */

#include <muse/module_service.hpp>
#include <muse/bounds_container.hpp>
#include <muse/convex_hull_container.hpp>
#include <muse/primitive_touch.hpp>
#include <muse/autoconfiguration.hpp>
#include <muse/filter.hpp>
#include <muse/apply.hpp>
#include <kerat/parsers.hpp>
#include <dtuio/dtuio.hpp>
#include <muse/recognizers/libreco.hpp>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <errno.h>
#include <limits>
#include <uuid/uuid.h>

#include "config_commons_internal.hpp"

namespace muse {
    libkerat::internals::convertor_list get_muse_convertors(){
        using libkerat::internals::convertor_list;
        convertor_list retval;
        
        { // libkerat
            convertor_list libkerat_convertors(libkerat::internals::get_libkerat_convertors());
            std::copy(libkerat_convertors.begin(), libkerat_convertors.end(), std::back_inserter(retval));
        }

        { // dtuio
            convertor_list dtuio_convertors(dtuio::internals::get_dtuio_convertors());
            std::copy(dtuio_convertors.begin(), dtuio_convertors.end(), std::back_inserter(retval));
        }
        
        return retval;
    }
}

template <class CONTAINER>
inline static int register_container(){
    return muse::module_service::get_instance()->register_module_container(CONTAINER::PATH, new CONTAINER);
}

template <class CONTAINER>
inline static void unregister_container(){
    muse::module_container * container = muse::module_service::get_instance()->unregister_module_container(CONTAINER::PATH);
    if (container != NULL){ delete container; }
}

class libkerat_adaptor_multiplexing: public muse::module_container {
public:
    //! \todo make module configurable
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config __attribute__((unused))) const {
        if (module == NULL){ return -1; }
        *module = new libkerat::adaptors::multiplexing_adaptor;
        return (*module == NULL);
    }
    static const char * PATH;
};
const char * libkerat_adaptor_multiplexing::PATH = "/libkerat/multiplexing_adaptor";

class libkerat_adaptor_scaling: public muse::module_container {
public:
    //! \todo make module configurable
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {
        if (module == NULL){ return -1; }
        *module = NULL;
        if (module_config == NULL){ return -1; }

        long tmp_width = 0, tmp_height = 0;
        long scale_accel = false;
        double x_scaling = 1, y_scaling = 1, z_scaling = 1;

        bool has_width = config_key_to_long(module_config, "width", tmp_width),
            has_height = config_key_to_long(module_config, "height", tmp_height),
            has_accel =  config_key_to_long(module_config, "scale_accel", scale_accel),
            has_x = config_key_to_double(module_config, "x_scale", x_scaling),
            has_y = config_key_to_double(module_config, "y_scale", y_scaling),
            has_z = config_key_to_double(module_config, "z_scale", z_scaling);

        libkerat::dimmension_t width = tmp_width, height = tmp_height;

        if (has_width && has_height){
            if (has_accel){
                *module = new libkerat::adaptors::scaling_adaptor(width, height, scale_accel);
            } else {
                *module = new libkerat::adaptors::scaling_adaptor(width, height);
            }
        } else if (has_x && has_y){
            if (!has_z){
                *module = new libkerat::adaptors::scaling_adaptor(x_scaling, y_scaling);
            } else {
                if (has_accel){
                    *module = new libkerat::adaptors::scaling_adaptor(x_scaling, y_scaling, z_scaling, scale_accel);
                } else {
                    *module = new libkerat::adaptors::scaling_adaptor(x_scaling, y_scaling, z_scaling);
                }
            }
        }

        return ((*module) == NULL);
    }
    static const char * PATH;
};
const char * libkerat_adaptor_scaling::PATH = "/libkerat/scaling_adaptor";

class dtuio_adaptor_marker: public muse::module_container {
public:
    //! \todo make module configurable
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {

        std::string mode;
        config_key_to_string(module_config, "coordinate_translation", mode);

        std::string purpose;
        config_key_to_string(module_config, "purpose", purpose);
        
        std::transform(mode.begin(), mode.end(), mode.begin(), tolower);
        std::transform(purpose.begin(), purpose.end(), purpose.begin(), tolower);

        dtuio::sensor::sensor_properties::coordinate_translation_mode_t tmp_mode;
        if (mode.compare("setup_once") == 0){
            tmp_mode = dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE;
        } else if (mode.compare("setup_continuous") == 0){
            tmp_mode = dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS;
        } else if (mode.compare("intact") == 0){
            tmp_mode = dtuio::sensor::sensor_properties::COORDINATE_INTACT;
        } else {
            std::cerr << PATH << ": invalid or unset coordinate_translation, defaulting to \"" << mode << "\" (possible values are: intact, setup_once, setup_continuous)!" << std::endl;
            tmp_mode = dtuio::sensor::sensor_properties::COORDINATE_INTACT;
        }
        
        dtuio::sensor::sensor_properties::sensor_purpose_t tmp_purpose;
        if (purpose.compare("source") == 0){
            tmp_purpose = dtuio::sensor::sensor_properties::PURPOSE_EVENT_SOURCE;
        } else if (purpose.compare("observer") == 0){
            tmp_purpose = dtuio::sensor::sensor_properties::PURPOSE_OBSERVER;
        } else if (purpose.compare("tagger") == 0){
            tmp_purpose = dtuio::sensor::sensor_properties::PURPOSE_TAGGER;
        } else {
            std::cerr << PATH << ": purpose invalid or unset, defaulting to \"" << purpose << "\" (possible values are: source, observer, tagger)!" << std::endl;
            tmp_purpose = dtuio::sensor::sensor_properties::PURPOSE_EVENT_SOURCE;
        }
        
        *module = new dtuio::adaptors::marker(tmp_mode, tmp_purpose);

        return ((*module) == NULL);
    }
    static const char * PATH;
};
const char * dtuio_adaptor_marker::PATH = "/dtuio/marker";

class dtuio_adaptor_viewport: public muse::module_container {
public:
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {

        std::string uuid_string;
        bool strip = false;
        bool has_uuid = config_key_to_string(module_config, "uuid", uuid_string);
        bool has_strip = config_key_to_bool(module_config, "strip", strip);

        dtuio::uuid_t uuid;
        uuid_clear(uuid);
        if (has_uuid) { uuid_parse(uuid_string.c_str(), uuid); }

        
        const TiXmlElement * viewport_config = module_config->FirstChildElement("viewport");
        bool has_viewport = viewport_config != NULL;
        if (has_viewport) {
            std::string center;
            bool has_center = config_key_to_string(viewport_config, "center", center);

            long width = 0;
            bool has_width = config_key_to_long(viewport_config, "width", width);
            long height = 0;
            bool has_height = config_key_to_long(viewport_config, "height", height);
            long depth = 0;
            bool has_depth = config_key_to_long(viewport_config, "depth", depth);

            double yaw = 0;
            config_key_to_double(viewport_config, "yaw", yaw);
            double pitch = 0;
            config_key_to_double(viewport_config, "pitch", pitch);
            double roll = 0;
            config_key_to_double(viewport_config, "roll", roll);
            
            if (!((has_width && has_depth) || (has_height && has_depth) || (has_height && has_depth))){
                std::cerr << PATH << ": At least two of (width, height, depth) have to be specified!" << std::endl;
                return false;
            }
            
            float coords[3];
            memset(coords, 0, sizeof(coords));

            if (has_center) { // parse viewport center
                std::vector<char> buff_vec;
                buff_vec.resize(center.size()+1);
                char * buffer = buff_vec.data();
                memcpy(buffer, center.c_str(), center.size() +1);

                char * input = buffer;
                for (size_t i = 0; i < 3; ++i){
                    char * component = strtok(input, " \t");
                    input = NULL;

                    char * endptr = NULL;
                    if ((i < 2) && (component == NULL)){ goto invalid; }
                    // z is voluntary
                    if ((i == 2) && (component == NULL)){ continue; }

                    coords[i] = strtof(component, &endptr);
                    if (*endptr != '\0'){ goto invalid; }

                    continue;

                invalid:
                    std::cerr << PATH << ": Invalid format for center!" << std::endl;
                    return false;
                }
            } else {
                std::cerr << PATH << ": Center not found, defaulting to center of the box!" << std::endl;
                coords[0] = width/2;
                coords[1] = height/2;
                coords[2] = depth/2;
            }
            
            dtuio::sensor::viewport vpt(
                uuid, 
                libkerat::helpers::point_3d(coords[0], coords[1], coords[2]),
                libkerat::helpers::angle_3d(yaw, pitch, roll),
                width, height, depth
            );
            
            if (has_strip){
                *module = new dtuio::adaptors::viewport_projector(vpt, strip);
            } else {
                *module = new dtuio::adaptors::viewport_projector(vpt);
            }
        } else {
            if (has_strip) {
                *module = new dtuio::adaptors::viewport_projector(uuid, strip);
            } else {
                *module = new dtuio::adaptors::viewport_projector(uuid);
            }
        }

        return ((*module) == NULL);
    }
    static const char * PATH;
};
const char * dtuio_adaptor_viewport::PATH = "/dtuio/viewport";

class dtuio_adaptor_scaler: public muse::module_container {
public:
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {

        std::string uuid_string;
        bool has_uuid = config_key_to_string(module_config, "uuid", uuid_string);

        dtuio::uuid_t uuid;
        uuid_clear(uuid);
        if (has_uuid) { uuid_parse(uuid_string.c_str(), uuid); }
        
        long width = 0;
        bool has_width = config_key_to_long(module_config, "width", width);
        long height = 0;
        bool has_height = config_key_to_long(module_config, "height", height);
        long depth = 0;
        bool has_depth = config_key_to_long(module_config, "depth", depth);

        if (!(has_width || has_depth|| has_height)){
            std::cerr << PATH << ": At least one of (width, height, depth) should be specified!" << std::endl;
        }

        dtuio::sensor::viewport vpt(uuid, width, height, depth);
        *module = new dtuio::adaptors::viewport_scaler(vpt);

        return ((*module) == NULL);
    }
    static const char * PATH;
};
const char * dtuio_adaptor_scaler::PATH = "/dtuio/scaler";

class muse_aggregator_cb: public muse::module_container {
public:
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {
        if (module == NULL){ return -1; }
        *module = NULL;
        if (module_config == NULL){ return -1; }

        libkerat::slot_t container_slot = 0;
        long tmp_container_slot = 0;
        std::string matching_regex;
        bool cascade = false;

        bool has_regex =   config_key_to_string(module_config, "matching_regex", matching_regex);
        bool has_slot =    config_key_to_long(module_config, "container_slot", tmp_container_slot);
        config_attr_to_bool(module_config, "cascade", cascade);
        //! \todo fix 2^n possible settings
        
        container_slot = tmp_container_slot;

        if (has_regex && has_slot){
            *module = new muse::aggregators::bounds_container(matching_regex, container_slot, cascade);
        } else if (has_slot){
            *module = new muse::aggregators::bounds_container(".*", container_slot, cascade);
        } else if (has_regex){
            *module = new muse::aggregators::bounds_container(matching_regex);
        }

        return (*module == NULL);
    }
    static const char * PATH;
};
const char * muse_aggregator_cb::PATH = "/muse/aggregator/container_bounds";

class muse_aggregator_cch: public muse::module_container {
public:
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {
        if (module == NULL){ return -1; }
        *module = NULL;
        if (module_config == NULL){ return -1; }

        libkerat::slot_t container_slot = 0;
        long tmp_container_slot = 0;
        std::string matching_regex;
        bool cascade = false;

        bool has_regex =   config_key_to_string(module_config, "matching_regex", matching_regex);
        bool has_slot =    config_key_to_long(module_config, "container_slot", tmp_container_slot);
        config_attr_to_bool(module_config, "cascade", cascade);
        //! \todo fix 2^n possible settings
        
        container_slot = tmp_container_slot;

        if (has_regex && has_slot){
            *module = new muse::aggregators::convex_hull_container(matching_regex, container_slot, cascade);
        } else if (has_slot){
            *module = new muse::aggregators::convex_hull_container(".*", container_slot, cascade);
        } else if (has_regex){
            *module = new muse::aggregators::convex_hull_container(matching_regex);
        }

        return (*module == NULL);
    }
    static const char * PATH;
};
const char * muse_aggregator_cch::PATH = "/muse/aggregator/container_convex_hull";

class muse_aggregator_filter: public muse::module_container {
public:
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {
        if (module == NULL){ return -1; }
        *module = NULL;
        if (module_config == NULL){ return -1; }

        std::string matching_regex;

        //! \todo add sink option to the module
        bool has_regex = config_key_to_string(module_config, "matching_regex", matching_regex);

        if (has_regex){
            *module = new muse::aggregators::filter(matching_regex);
        } else {
            return -1;
        }

        return (*module == NULL);
    }
    static const char * PATH;
};
const char * muse_aggregator_filter::PATH = "/muse/aggregator/filter";

class muse_aggregator_apply: public muse::module_container {
public:
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {
        if (module == NULL){ return -1; }
        *module = NULL;
        if (module_config == NULL){ return -1; }

        std::string matching_regex;
        const TiXmlElement * e_chain;

        //! \todo add sink option to the module
        bool has_regex = config_key_to_string(module_config, "matching_regex", matching_regex);
        bool has_chain = config_key_to_element(module_config, "chain", &e_chain);
        
        if (has_regex && has_chain){
            muse::module_service::module_chain modules;
            muse::aggregators::apply::adaptor_vector adaptors;
            
            muse::module_service * modserv = muse::module_service::get_instance();
            
            if (modserv->create_module_chain(e_chain, modules)){
                std::cerr << "Failed to create module chain for " << PATH << std::endl;
                modserv->free_module_chain(modules);
                return -1;
            }

            if (modules.empty()){
                return -2;
            }
                
            for (
                muse::module_service::module_chain::iterator module_instance = modules.begin();
                module_instance != modules.end();
                module_instance++
            ){
                adaptors.push_back(module_instance->first);
            }
            
            //! \todo possible memory leak of configuration from modules
            
            *module = new muse::aggregators::apply(matching_regex, adaptors, true);
        } else {
            return -1;
        }

        return (*module == NULL);
    }
    static const char * PATH;
};
const char * muse_aggregator_apply::PATH = "/muse/aggregator/apply";

class muse_sensor_pt: public muse::module_container {
public:
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {
        if (module == NULL){ return -1; }
        *module = NULL;
        if (module_config == NULL){ return -1; }

        uint32_t join_treshold = 0;
        libkerat::timetag_t timetag; memset(&timetag, 0, sizeof(libkerat::timetag_t));
        double tmp_timetag = 0;

        long tmp_join_treshold = 0;
        bool has_treshold = config_key_to_long(module_config, "treshold", tmp_join_treshold);
        bool has_timetag = config_key_to_double(module_config, "timeout", tmp_timetag);

        timetag.sec = tmp_timetag;
        tmp_timetag -= timetag.sec;
        tmp_timetag *= 2<<16;
        tmp_timetag *= 2<<16;
        timetag.frac = tmp_timetag;
        join_treshold = tmp_join_treshold;

        if (has_treshold && has_timetag){
            *module = new muse::virtual_sensors::primitive_touch(join_treshold, timetag);
        } else {
            *module = new muse::virtual_sensors::primitive_touch;
        }

        return ((*module) == NULL);
    }
    static const char * PATH;
};
const char * muse_sensor_pt::PATH = "/muse/sensor/primitive_touch";

class muse_sensor_autoconfiguration: public muse::module_container {
public:
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config) const {
        bool cut_received = true;
        bool has_cut = false;
 
        if (module_config != NULL) { has_cut = config_key_to_bool(module_config, "cut_received", cut_received); }
        if (!has_cut) {
            cut_received = true;
            std::cerr << PATH << ": invalid value for cut_received, defaulting to \"" << cut_received << "\"!" << std::endl;
        }
        
        std::string mode;
        if (module_config != NULL){
            config_key_to_string(module_config, "coordinate_translation", mode);
        }

        std::transform(mode.begin(), mode.end(), mode.begin(), tolower);

        dtuio::sensor::sensor_properties::coordinate_translation_mode_t tmp_mode;
        if (mode.compare("setup_once") == 0){
            tmp_mode = dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE;
        } else if (mode.compare("setup_continuous") == 0){
            tmp_mode = dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS;
        } else if (mode.compare("intact") == 0){
            tmp_mode = dtuio::sensor::sensor_properties::COORDINATE_INTACT;
        } else {
            std::cerr << PATH << ": invalid or unset coordinate_translation, defaulting to \"" << mode << "\" (possible values are: intact, setup_once, setup_continuous)!" << std::endl;
            tmp_mode = dtuio::sensor::sensor_properties::COORDINATE_INTACT;
        }
        
        
        *module = new muse::virtual_sensors::autoremapper(cut_received, tmp_mode);

        return ((*module) == NULL);
    }
    static const char * PATH;
};
const char * muse_sensor_autoconfiguration::PATH = "/muse/sensor/autoconfiguration";

// here the libreco xml parsers and modules begins

using libkerat::helpers::point_2d;
using libreco::rutils::gesture_identity;

using std::vector;
using std::map;
using std::pair;
using std::stringstream;

static bool libreco_parse_points(const char * text, vector<point_2d> & stroke) {
    std::string error_details = "Error parsing points from text!\n";
	
	if(text == NULL) {
		std::cerr << error_details;
		return false;
	}
    //there must be at least x_coordinate y_coordinate and space between them
	if (strlen(text) < 3) {
		std::cerr << error_details;
		return false;
    }
    stringstream content;
    content << text;

    int x_coord;
    int y_coord;
    bool even;

    while (content.good()) {
        even = false;
        if (content >> x_coord) {
            if (content >> y_coord) {
                //stores loaded point to output vector
                point_2d p(x_coord, y_coord);
                stroke.push_back(p);
                even = true;
            }
        }
        //two coordinates must be loaded
        if (!even) {
            stroke.clear();
			std::cerr << error_details;
		    return false;
        }
    }
    return true;
}

static bool libreco_load_id_attr(const TiXmlElement * element, const char * attribute_name, uint16_t & id) {
    long tmp_id = 0;
    bool has_id = config_attr_to_long(element, attribute_name, tmp_id);
    
    if ((!has_id) || (tmp_id < 0) || (tmp_id > std::numeric_limits<uint16_t>::max())){
        std::cerr << "Invalid or not present gesture ID or stroke ID! Skipping over the gesture!" << std::endl;
        std::cerr << "Correct format: gesture_id=\"uint16_t\" or in case of stroke: stroke_id=\"uint16_t\"" << std::endl;
        return false;
    }
    id = tmp_id;

	return has_id;
}

static bool libreco_load_gest_name_attr(const TiXmlElement * element, std::string & name) {
    bool retval = config_attr_to_string(element, "name", name);
    if (!retval){
        std::cerr << "Error loading gesture name! Skipping over the gesture!" << std::endl;
        return false;
    }
    
    //empty name is not a very good idea
    if(!name.length()) {
        std::cerr << "Error loading gesture name! Name empty! Skipping over the gesture!" << std::endl;
        return false;
    }
    
    return true;
}

static bool libreco_unistroke_parser(const TiXmlElement * e_uni_gesture, gesture_identity & gest_iden, vector<point_2d> & stroke) {
	std::string error_separator = "=========================================================\n";

	//load gesture_id attribute
	uint16_t gest_id;
	if(!(libreco_load_id_attr(e_uni_gesture, "gesture_id", gest_id))) {
		std::cerr << error_separator << std::endl;
		return false;
	}

    //load name attribute
    std::string gest_name;
    if (!(libreco_load_gest_name_attr(e_uni_gesture, gest_name))) {
        std::cerr << "TEMPLATE_ID: " << gest_id << std::endl << error_separator;
        return false;
    }
    
    stringstream error_details("");
    error_details << "TEMPLATE_ID: " << gest_id << std::endl << "TEMPLATE_NAME: " << gest_name << std::endl << error_separator;
    
    //load sensitivity attribute
	bool sensitivity;
	if(!config_attr_to_bool(e_uni_gesture, "sensitivity", sensitivity)) {
		std::cerr << "Warning (IGNORE in case dollar one ($1) is used). Sensitivity attribute not set or invalid! Defaulting to false!";
        std::cerr << std::endl << error_details.str();
        sensitivity = false;
	}

    //load revert attribute
	bool tmp_revert;
    if(!config_attr_to_bool(e_uni_gesture, "revert", tmp_revert)) {
   		std::cerr << "Warning (IGNORE in case dollar N ($N) is used). Revert attribute not set or invalid! Defaulting to false!";
        std::cerr << std::endl << error_details.str();
        tmp_revert = false;
    }

    //load gesture points
    if (!libreco_parse_points(e_uni_gesture->GetText(), stroke)) {
        std::cerr << "Skipping over the gesture!" << std::endl;
        std::cerr << "Correct format of input points should be <x_coordinate><tabulator OR space><y_coordinate><new line>" << std::endl;
   		std::cerr << error_details.str();
        return false;
    }

    //if everything was done without errors, information are stored
    gest_iden.gesture_name = gest_name;
    gest_iden.gesture_id = gest_id;
    gest_iden.orientation_sensitive = sensitivity;
    gest_iden.revert = tmp_revert;

    return true;
}

typedef map<gesture_identity, vector<point_2d> > libreco_generic_singlestroke_map;
typedef map<gesture_identity, vector<vector<point_2d> > > libreco_generic_multistroke_map;

static bool libreco_unistroke_loader(const TiXmlElement * e_config, libreco_generic_singlestroke_map & loaded_templates) {
    
	//holds loaded templates
    loaded_templates.clear();
    pair < libreco_generic_singlestroke_map::const_iterator, bool> insert_pair;

    vector<point_2d> unistroke;
    gesture_identity gest_iden;

    const TiXmlElement * e_uni_gesture = e_config->FirstChildElement("uni_gesture");

    while (e_uni_gesture != NULL) {
		//new iteration, erase all previously loaded points
        unistroke.clear();

        if (libreco_unistroke_parser(e_uni_gesture, gest_iden, unistroke)) {
            insert_pair = loaded_templates.insert(pair<gesture_identity, vector<point_2d> >(gest_iden, unistroke));

            //warning if gesture with the same name was already loaded
            if (!insert_pair.second) {
  			 	std::cerr << "Warning: libreco_unistroke_loader gesture redefinition! Skipping over the gesture!" << std::endl;
                std::cerr << "CAUSE: more than one template with given id was found. First one stored, all others skipped!" << std::endl;
                std::cerr << "TEMPLATE_ID: " << gest_iden.gesture_id << std::endl << "TEMPLATE_NAME: " << gest_iden.gesture_name << std::endl;
            }
        }
        e_uni_gesture = e_uni_gesture->NextSiblingElement("uni_gesture");
    }

#if 0
    //Output for testing purpose (will be removed)
    std::cout << "Loaded uni-stroke gestures:" << std::endl;
    for (libreco_generic_singlestroke_map::const_iterator iter = loaded_templates.begin();
            iter != loaded_templates.end(); iter++) {

        std::cout << "id          : " << iter->first.gesture_id << std::endl;
        std::cout << "name        : " << iter->first.gesture_name << std::endl;
        std::cout << "sensitivity : " << ((iter->first.orientation_sensitive) ? "true" : "false") << std::endl;
        std::cout << "revert      : " << ((iter->first.revert) ? "true" : "false") << std::endl;
        std::cout << "points      : " << std::endl;

        for (vector<point_2d>::const_iterator iter2 = iter->second.begin();
                iter2 != iter->second.end(); iter2++) {

            std::cout << iter2->get_x() << "|" << iter2->get_y() << std::endl;
        }
        std::cout << "*****************************************" << std::endl;
    }
	//End of Output
#endif
        
    return !loaded_templates.empty();
}

bool libreco_anystrokes_loader(const TiXmlElement * e_config, libreco_generic_multistroke_map & loaded_templates) {
    
	//holds loaded templates
    loaded_templates.clear();
    pair < libreco_generic_multistroke_map::const_iterator, bool> insert_pair;

    libreco_generic_multistroke_map::mapped_type multi_strokes;
    vector<point_2d> unistroke;
    
	//in the first place go through the uni_gesture tags and load uni-stroke templates
    const TiXmlElement * e_uni_gesture = e_config->FirstChildElement("uni_gesture");

    while (e_uni_gesture != NULL) {
		//clear all previously loaded uni-strokes and multi-strokes
        unistroke.clear();
        multi_strokes.clear();
        
        //create temporary gesture identity structure
		gesture_identity temp_gest_iden;
        if (libreco_unistroke_parser(e_uni_gesture, temp_gest_iden, unistroke)) {
            //loaded stroke (vector) is wrapped with another vector (because multi-stroke gestures are stored that way)
            multi_strokes.push_back(unistroke);
            insert_pair = loaded_templates.insert(libreco_generic_multistroke_map::value_type(temp_gest_iden, multi_strokes));
            
            if (!insert_pair.second) {
  			 	std::cerr << "Warning: libreco_anystrokes_loader gesture redefinition! Skipping over the gesture!" << std::endl;
                std::cerr << "CAUSE: more than one template with given id was found. First one stored, all others skipped!" << std::endl;
                std::cerr << "TEMPLATE_ID: " << temp_gest_iden.gesture_id << std::endl << "TEMPLATE_NAME: " << temp_gest_iden.gesture_name << std::endl;
            }
        }
        e_uni_gesture = e_uni_gesture->NextSiblingElement("uni_gesture");
    }
    
    /*at this point all uni-stroke templates have been loaded*/
	
	std::string error_separator = "=========================================================\n";
    
	map<uint16_t, vector<point_2d> > partial_strokes;
    pair<map<uint16_t, vector<point_2d> >::const_iterator, bool> partial_insert_pair;
    
    //next go through the multi_gesture tags and load multi-strokes templates
    const TiXmlElement * e_multi_gesture = e_config->FirstChildElement("multi_gesture");
    
	while (e_multi_gesture != NULL) {
        //clear all previously loaded multi-strokes and partial strokes belonging to multi-stroke gestures
        multi_strokes.clear();
		partial_strokes.clear();
        
        //create temporary gesture identity structure
		gesture_identity temp_gest_iden;
		
		//load gesture_id attribute
		uint16_t gest_id;
		if(!(libreco_load_id_attr(e_multi_gesture, "gesture_id", gest_id))) {
			std::cerr << error_separator << std::endl;
            e_multi_gesture = e_multi_gesture->NextSiblingElement("multi_gesture");
            continue;
		}
		temp_gest_iden.gesture_id = gest_id;

    	//load name attribute
        std::string gest_name;
    	if (!(libreco_load_gest_name_attr(e_multi_gesture, gest_name))) {
        	std::cerr << "TEMPLATE_ID: " << gest_id << std::endl << error_separator;
			e_multi_gesture = e_multi_gesture->NextSiblingElement("multi_gesture");
            continue;
    	}
        temp_gest_iden.gesture_name = gest_name;

	    //load sensitivity attribute
		bool sensitivity;
		if(!config_attr_to_bool(e_multi_gesture, "sensitivity", sensitivity)) {
			std::cerr << "TEMPLATE_ID: " << gest_id << std::endl << "TEMPLATE_NAME: " << gest_name << std::endl << error_separator;
			sensitivity = false;
		}
		temp_gest_iden.orientation_sensitive = sensitivity;
		
		//load partial strokes of multi-stroke gesture
        const TiXmlElement * e_stroke = e_multi_gesture->FirstChildElement("stroke");
        bool correctly_loaded = true;
        
        while (e_stroke != NULL) {
			//clear previously loaded partial stroke
			unistroke.clear();

			//load stroke_id attribute
			uint16_t str_id;
			if(!(libreco_load_id_attr(e_stroke, "stroke_id", str_id))) {
				correctly_loaded = false;
				break;
			}
			
			//load stroke points
            if (!libreco_parse_points(e_stroke->GetText(), unistroke)) {
				correctly_loaded = false;
                break;
            }
			
			//insert stroke to temporary partial stroke map
            partial_insert_pair = partial_strokes.insert(std::pair<uint16_t, vector<point_2d> >(str_id, unistroke));

            //if stroke with given id already exists, partial stroke is skipped
            if (!partial_insert_pair.second) {
  			 	std::cerr << "Warning: child tag <stroke> with stroke_id=\"" << str_id << "\" redefinition! Skipping over this partial stroke!" << std::endl;
                std::cerr << "CAUSE: multiple partial strokes of one multi-stroke gesture have got the same id. First one stored all others skipped!" << std::endl;
                std::cerr << "TEMPLATE_ID: " << gest_id << std::endl << "TEMPLATE_NAME: " << gest_name << std::endl;
			}

            e_stroke = e_stroke->NextSiblingElement("stroke");
        }

		//if partial strokes were not loaded correctly, template is skipped
        if (!correctly_loaded) {
  			std::cerr << "TEMPLATE_ID: " << gest_id << std::endl << "TEMPLATE_NAME: " << gest_name << std::endl << error_separator;
			e_multi_gesture = e_multi_gesture->NextSiblingElement("multi_gesture");
            continue;
        }
        
        //if no partial stroke was loaded
        if(partial_strokes.empty()) {
            std::cerr << "Warning: no partial strokes of multi-stroke gesture present! Skipping over the gesture!" << std::endl;
            std::cerr << "TEMPLATE_ID: " << gest_id << std::endl << "TEMPLATE_NAME: " << gest_name << std::endl << error_separator;
			e_multi_gesture = e_multi_gesture->NextSiblingElement("multi_gesture");
            continue;
        }
        
        /*if parser reaches this point everything went wonderful and new multi-stroke template can be created/stored*/
		
		//copy loaded strokes, stroke id is no more needed
		for(map<uint16_t, vector<point_2d> >::const_iterator iter = partial_strokes.begin(); iter != partial_strokes.end(); iter++) {
			multi_strokes.push_back(iter->second);
		}
		
        //create new template
        insert_pair = loaded_templates.insert(pair<gesture_identity, vector<vector<point_2d> > >(temp_gest_iden, multi_strokes));
        if (!insert_pair.second) {
            std::cerr << "Warning: libreco_anystrokes_loader gesture redefinition! Skipping over the gesture!" << std::endl;
            std::cerr << "CAUSE: more than one template with given id was found. First one stored, all others skipped!" << std::endl;
            std::cerr << "TEMPLATE_ID: " << temp_gest_iden.gesture_id << std::endl << "TEMPLATE_NAME: " << temp_gest_iden.gesture_name << std::endl;
        }

        e_multi_gesture = e_multi_gesture->NextSiblingElement("multi_gesture");
    }

#if 0
    //Output for testing purpose (will be removed)
    std::cout << "Loaded multi and uni stroke gestures" << std::endl;
    for (map<gesture_identity, vector<vector<point_2d> > >::const_iterator iter = loaded_templates.begin();
            iter != loaded_templates.end(); iter++) {

		std::cout << "id          : " << iter->first.gesture_id << std::endl;
		std::cout << "name        : " << iter->first.gesture_name << std::endl;
        std::cout << "sensitivity : " << ((iter->first.orientation_sensitive) ? "true" : "false") << std::endl;
        std::cout << "revert      : " << ((iter->first.revert) ? "true" : "false") << std::endl;
        std::cout << "points      : " << std::endl;

        for (vector<vector<point_2d> >::const_iterator iter2 = iter->second.begin();
                iter2 != iter->second.end(); iter2++) {

            std::cout << "stroke:   " << std::endl;
            for (vector<point_2d>::const_iterator iter3 = iter2->begin(); iter3 != iter2->end(); iter3++) {
                std::cout << iter3->get_x() << "|" << iter3->get_y() << std::endl;
            }

        }
        std::cout << "*****************************************" << std::endl;
    }
	//End of output
#endif
    
    return !loaded_templates.empty();
}

// here the libreco modules begin

static bool libreco_load_uint16_key(const TiXmlElement * module_config, const char * key_name, uint16_t & value) {
    long tmp_value = 0;
    bool has_points_count = config_key_to_long(module_config, key_name, tmp_value);
    
    if((!has_points_count) || (tmp_value < 0) || (tmp_value > std::numeric_limits<uint16_t>::max())) {
        std::cerr << "Error loading module configuration - " << key_name << " not set or invalid. ";
    }
    
    value = tmp_value;
    return has_points_count;
}

static bool libreco_load_uint32_key(const TiXmlElement * module_config, const char * key_name, uint32_t & value) {
    long tmp_value = 0;
    bool has_points_count = config_key_to_long(module_config, key_name, tmp_value);
    
    if((!has_points_count) || (tmp_value < 0) || ((uint32_t)tmp_value > std::numeric_limits<uint32_t>::max())) {
        std::cerr << "Error loading module configuration - " << key_name << " not set or invalid. ";
    }
    
    value = tmp_value;
    return has_points_count;
}

static bool libreco_load_origin_key(const TiXmlElement * module_config, libkerat::helpers::point_2d & origin) {
    std::string origin_str = "0 0";
    bool has_origin = config_key_to_string(module_config, "origin", origin_str);
    if (!has_origin) {
        std::cerr << "Error loading module configuration - origin unset! ";
    }
    
    vector<point_2d> origin_points;
    libreco_parse_points(origin_str.c_str(), origin_points);
    if (origin_points.size() != 1){
        std::cerr << "Error loading module configuration - origin has to contain exactly one point. ";
        origin_points.clear();
        origin_points.push_back(point_2d(0, 0));
    }
    
    origin = origin_points.front();
    return has_origin;
}

static libreco::rutils::unistroke_gesture libreco_convert_singlestroke(const libreco_generic_singlestroke_map::value_type & original){
    libreco::rutils::unistroke_gesture gesture;
    gesture.name = original.first.gesture_name;
    gesture.revert = original.first.revert;
    gesture.sensitive = original.first.orientation_sensitive;
    gesture.points = original.second;
    return gesture;
}

class libreco_adaptor_dollar_n: public muse::module_container {
public:
    //! \todo make module configurable
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config __attribute__((unused))) const {
        if (module == NULL){ return -1; }
        
        // load templates
        libreco_generic_multistroke_map gestures;
        libreco_anystrokes_loader(module_config, gestures);
        
        //load dollar_n constructor attributes
        uint16_t resample_count = 0;
        if(!libreco_load_uint16_key(module_config, "resample", resample_count)) {
            std::cerr << "Dollar N ($N) resample count set to default value = 96" << std::endl;
            resample_count = 96;
        }
        
        point_2d origin;
        if(!libreco_load_origin_key(module_config, origin)) {
            std::cerr << "Dollar N ($N) default origin set to (0, 0)" << std::endl;
        }
        
        float rot_down = 0.0;
        if(!config_key_to_float(module_config, "rotation_down_limit", rot_down)) {
            std::cerr << "Error loading module configuration - rotation_down_limit invalid or unset! ";
            std::cerr << "Dollar N ($N) default rotation_down_limit value set to -45°" << std::endl;
            rot_down = -45;
        }
        
        float rot_up = 0.0;
        if(!config_key_to_float(module_config, "rotation_up_limit", rot_up)) {
            std::cerr << "Error loading module configuration - rotation_up_limit invalid or unset! ";
            std::cerr << "Dollar N ($N) default rotation_up_limit value set to 45°" << std::endl;
            rot_up = 45;
        }
        
        float rot_thresh = 0.0;
        if(!config_key_to_float(module_config, "rotation_threshold", rot_thresh)) {
            std::cerr << "Error loading module configuration - rotation_threshold invalid or unset! ";
            std::cerr << "Dollar N ($N) default rotation_threshold value set to 2°" << std::endl;
            rot_thresh = 2;
        }
        
        uint16_t scale_box = 0;
        if(!libreco_load_uint16_key(module_config, "scale_box_size", scale_box)) {
            std::cerr << "Dollar N ($N) scale_box_size set to default value = 250" << std::endl;
            scale_box = 250;
        }
        
        float scale_thresh = 0.0;
        if(!config_key_to_float(module_config, "scaling_threshold", scale_thresh)) {
            std::cerr << "Error loading module configuration - scaling_threshold invalid or unset! ";
            std::cerr << "Dollar N ($N) default scaling_threshold value set to 0.30" << std::endl;
            scale_thresh = 0.30;
        }
        
        uint16_t start_index = 0;
        if(!libreco_load_uint16_key(module_config, "start_vector_index", start_index)) {
            std::cerr << "Dollar N ($N) start_vector_index set to default value = 12" << std::endl;
            start_index = 12;
        }
        
        float start_thresh = 0.0;
        if(!config_key_to_float(module_config, "start_vector_threshold", start_thresh)) {
            std::cerr << "Error loading module configuration - start_vector_threshold invalid or unset! ";
            std::cerr << "Dollar N ($N) default start_vector_threshold value set to 30°" << std::endl;
            start_thresh = 30;
        }
        
        bool eq_strokes = false;
        if(!config_key_to_bool(module_config, "equal_strokes_numbers", eq_strokes)) {
            std::cerr << "Error loading module configuration - equal_strokes_numbers invalid or unset! ";
            std::cerr << "Dollar N ($N) default equal_strokes_numbers value set to true" << std::endl;
            eq_strokes = true;
        }
        
        //load multistroke_adaptor attributes
        uint32_t timeout_sec = 0;
        if(!libreco_load_uint32_key(module_config, "timeout_seconds", timeout_sec)) {
            std::cerr << "Multistroke adaptor timeout_seconds set to default value = 2" << std::endl;
            timeout_sec = 2;
        }
        
        uint32_t timeout_frac = 0;
        if(!libreco_load_uint32_key(module_config, "timeout_fraction", timeout_frac)) {
            std::cerr << "Multistroke adaptor timeout_fraction set to default value = 0" << std::endl;
            timeout_frac = 0;
        }
        
        uint32_t radius = 0;
        if(!libreco_load_uint32_key(module_config, "radius", radius)) {
            std::cerr << "Multistroke adaptor radius set to default value = 0" << std::endl;
            radius = 0;
        }
        
        
        //load uuid for sensor_properties message
        std::string uuid_string;
        bool has_uuid = config_key_to_string(module_config, "uuid", uuid_string);

        dtuio::uuid_t uuid;
        uuid_clear(uuid);
        if ((!has_uuid) || (uuid_parse(uuid_string.c_str(), uuid) != 0)) {
            std::cerr << "Multistroke adaptor sensor_properties message uuid unset or invalid." << std::endl;
        }
        
        libreco::recognizers::dollar_n tmp_dollar_n(gestures, resample_count, origin, rot_down, rot_up, rot_thresh,
                                                    scale_box, scale_thresh, start_index, start_thresh, eq_strokes);
        
                        
        *module = new libreco::adaptors::multistroke_adaptor<libreco::recognizers::dollar_n>(tmp_dollar_n, uuid, timeout_sec, timeout_frac, radius);
        
        return (*module == NULL);
    }
    static const char * PATH;
};
const char * libreco_adaptor_dollar_n::PATH = "/muse/recognizers/dollar_n";


class libreco_adaptor_protractor: public muse::module_container {
public:
    //! \todo make module configurable
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config __attribute__((unused))) const {
        if (module == NULL){ return -1; }
        
        // load templates
        libreco_generic_singlestroke_map gestures;
        libreco_unistroke_loader(module_config, gestures);

        uint16_t resample_count = 0;
        if(!libreco_load_uint16_key(module_config, "resample", resample_count)) {
            std::cerr << "Protractor resample count set to default value = 16" << std::endl;
            resample_count = 16;
        }
        
        point_2d origin;
        if(!libreco_load_origin_key(module_config, origin)) {
            std::cerr << "Protractor default origin set to (0, 0)" << std::endl;
        }
        
        vector<libreco::rutils::unistroke_gesture> tmp_gestures;
        std::transform(gestures.begin(), gestures.end(), std::back_inserter(tmp_gestures), libreco_convert_singlestroke);
        
        libreco::recognizers::protractor tmp_protractor(tmp_gestures, resample_count, origin);
        
        *module = new libreco::adaptors::unistroke_adaptor<libreco::recognizers::protractor>(tmp_protractor);
        return (*module == NULL);
    }
    static const char * PATH;
};
const char * libreco_adaptor_protractor::PATH = "/muse/recognizers/protractor";


class libreco_adaptor_dollar_recognizer: public muse::module_container {
public:
    //! \todo make module configurable
    virtual int create_module_instance(muse::muse_module ** module, const TiXmlElement * module_config __attribute__((unused))) const {
        if (module == NULL){ return -1; }
        
        // load templates
        libreco_generic_singlestroke_map gestures;
        libreco_unistroke_loader(module_config, gestures);
        
        uint16_t resample_count = 0;
        if(!libreco_load_uint16_key(module_config, "resample", resample_count)) {
            std::cerr << "Dollar one ($1) resample count set to default value = 64" << std::endl;
            resample_count = 64;
        }
        
        point_2d origin;
        if(!libreco_load_origin_key(module_config, origin)) {
            std::cerr << "Dollar one ($1) default origin set to (0, 0)" << std::endl;
        }
        
        float rot_down = 0.0;
        if(!config_key_to_float(module_config, "rotation_down_limit", rot_down)) {
            std::cerr << "Error loading module configuration - rotation_low_bound invalid or unset! ";
            std::cerr << "Dollar one ($1) default rotation_low_bound value set to -45°" << std::endl;
            rot_down = -(M_PI / 4.0);
        }
        
        float rot_up = 0.0;
        if(!config_key_to_float(module_config, "rotation_up_limit", rot_up)) {
            std::cerr << "Error loading module configuration - rotation_up_bound invalid or unset! ";
            std::cerr << "Dollar one ($1) default rotation_up_bound value set to 45°" << std::endl;
            rot_up = M_PI / 4.0;
        }
        
        float rot_thresh = 0.0;
        if(!config_key_to_float(module_config, "rotation_threshold", rot_thresh)) {
            std::cerr << "Error loading module configuration - threshold invalid or unset! ";
            std::cerr << "Dollar one ($1) default threshold value set to 2°" << std::endl;
            rot_thresh = M_PI / 90.0;
        }
        
        uint16_t scale_box = 0;
        if(!libreco_load_uint16_key(module_config, "scale_box_size", scale_box)) {
            std::cerr << "Dollar one ($1) side of scale_box set to default value = 250" << std::endl;
            scale_box = 250;
        }
        
        vector<libreco::rutils::unistroke_gesture> tmp_gestures;
        std::transform(gestures.begin(), gestures.end(), std::back_inserter(tmp_gestures), libreco_convert_singlestroke);
        
        libreco::recognizers::dollar_recognizer tmp_dollar_rec(tmp_gestures, resample_count, origin, rot_down, rot_up, rot_thresh, scale_box);
        
        *module = new libreco::adaptors::unistroke_adaptor<libreco::recognizers::dollar_recognizer>(tmp_dollar_rec);
        return (*module == NULL);
    }
    static const char * PATH;
};
const char * libreco_adaptor_dollar_recognizer::PATH = "/muse/recognizers/dollar_recognizer";

// here the libreco xml parsers and modules ends

muse::module_service * muse::module_service::s_ms_instance = NULL;

//! \brief convert libkerat adaptors to muse modules
int register_libkerat_modules();
void unregister_libkerat_modules();
//! \brief convert dtuio adaptors to muse modules
int register_dtuio_modules();
void unregister_dtuio_modules();
//! \brief convert libreco adaptors to muse modules
int register_libreco_modules();
void unregister_libreco_modules();
//! \brief convert muse adaptors to muse modules
int register_muse_modules();
void unregister_muse_modules();

namespace muse {
    namespace internals {
        int init() __attribute__((constructor));
        int finalize() __attribute__((destructor));
    }
}

//extern "C" {
//    int muse_init(){ return muse::internals::init(); }
//}

int muse::internals::init() {
    register_libkerat_modules();
    register_dtuio_modules();
    register_libreco_modules();
    register_muse_modules();
    return 0;
}
int muse::internals::finalize() {
    unregister_muse_modules();
    unregister_libreco_modules();
    unregister_dtuio_modules();
    unregister_libkerat_modules();
    return 0;
}

//! \brief convert libkerat adaptors to muse modules
int register_libkerat_modules() {
    int retval = 0;
    retval += register_container<libkerat_adaptor_multiplexing>();
    retval += register_container<libkerat_adaptor_scaling>();
    return retval;
}
void unregister_libkerat_modules() {
    unregister_container<libkerat_adaptor_multiplexing>();
    unregister_container<libkerat_adaptor_scaling>();
}

//! \brief convert dtuio adaptors to muse modules
int register_dtuio_modules() {
    int retval = 0;
    retval += register_container<dtuio_adaptor_marker>();
    retval += register_container<dtuio_adaptor_viewport>();
    retval += register_container<dtuio_adaptor_scaler>();
    return retval;
}
void unregister_dtuio_modules() {
    unregister_container<dtuio_adaptor_marker>();
    unregister_container<dtuio_adaptor_viewport>();
    unregister_container<dtuio_adaptor_scaler>();
}

//! \brief convert libreco adaptors to muse modules
int register_libreco_modules() {
    int retval = 0;
    retval += register_container<libreco_adaptor_dollar_n>();
    retval += register_container<libreco_adaptor_dollar_recognizer>();
    retval += register_container<libreco_adaptor_protractor>();
    return retval;
}
void unregister_libreco_modules() {
    unregister_container<libreco_adaptor_protractor>();
    unregister_container<libreco_adaptor_dollar_recognizer>();
    unregister_container<libreco_adaptor_dollar_n>();    
}

//! \brief convert muse components to muse modules
int register_muse_modules() {
    int retval = 0;
    retval += register_container<muse_aggregator_cb>();
    retval += register_container<muse_aggregator_cch>();
    retval += register_container<muse_sensor_pt>();
    retval += register_container<muse_sensor_autoconfiguration>();
    retval += register_container<muse_aggregator_filter>();
    retval += register_container<muse_aggregator_apply>();
    return retval;
}
void unregister_muse_modules() {
    unregister_container<muse_aggregator_cb>();
    unregister_container<muse_aggregator_cch>();
    unregister_container<muse_sensor_pt>();
    unregister_container<muse_sensor_autoconfiguration>();
    unregister_container<muse_aggregator_apply>();
    unregister_container<muse_aggregator_filter>();
}

////////////////////////////////// THIS IS THE MOST IMPORTANT LINE IN THIS FILE!
