/**
 * @file      nodeconfig.cpp
 * @brief     Implements the struct that holds the wrapper configuration
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2012-11-22 02:15 UTC+1
 * @copyright BSD
 */

#include <fcntl.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string>

#include <tinyxml.h>
#include <uuid/uuid.h>
#include <dtuio/dtuio.hpp>

#include "nodeconfig.hpp"

libkerat::kerat_message * wrapper_config::cloner::operator()(const libkerat::kerat_message * original) const {
            return original->clone();
        }
libkerat::kerat_message * wrapper_config::destroyer::operator()(libkerat::kerat_message * original) const {
            delete original;
            return NULL;
        }
    
wrapper_config::wrapper_config()
    :x(0),y(0),top_left(0, 0),bottom_left(0, 0),bottom_right(0, 0),top_right(0, 0),
    virtual_sensor_width(DIMMENSION_UNSET),virtual_sensor_height(DIMMENSION_UNSET),local_ip(0),instance(0),disable_transformations(false),join_distance_limit(400)
{ ; }
    
wrapper_config::wrapper_config(const wrapper_config & original)
    :x(original.x),y(original.y),
    top_left(original.top_left),bottom_left(original.bottom_left),
    bottom_right(original.bottom_right),top_right(original.top_right),
    virtual_sensor_width(original.virtual_sensor_width),virtual_sensor_height(original.virtual_sensor_height),
    target_addr(original.target_addr), app_name(original.app_name),
    local_ip(original.local_ip),instance(original.instance),
    axes_mappings(original.axes_mappings), axes_ranges(original.axes_ranges),
    disable_transformations(original.disable_transformations),
    join_distance_limit(original.join_distance_limit)
{
    std::transform(
        original.prepared_dtuio.begin(), original.prepared_dtuio.end(), 
        std::back_inserter(prepared_dtuio), cloner()
    );
}
    
wrapper_config::~wrapper_config(){
    std::for_each(prepared_dtuio.begin(), prepared_dtuio.end(), destroyer());
    prepared_dtuio.clear();
}
    
wrapper_config & wrapper_config::operator=(const wrapper_config & original){
    if (this == &original){ return *this; }

    std::for_each(prepared_dtuio.begin(), prepared_dtuio.end(), destroyer());
    prepared_dtuio.clear();

    x = original.x;
    y = original.y;

    top_left = original.top_left;
    bottom_left = original.bottom_left;
    bottom_right = original.bottom_right;
    top_right = original.top_right;

    virtual_sensor_width = original.virtual_sensor_width;
    virtual_sensor_height = original.virtual_sensor_height;

    target_addr = original.target_addr;
    app_name = original.app_name;
    
    local_ip = original.local_ip;
    instance = original.instance;
    
    axes_mappings = original.axes_mappings;
    axes_ranges = original.axes_ranges;

    disable_transformations = original.disable_transformations;
    join_distance_limit = original.join_distance_limit;

    std::transform(
        original.prepared_dtuio.begin(), original.prepared_dtuio.end(), 
        std::back_inserter(prepared_dtuio), cloner()
    );

    return *this;
}

node_config::node_config(){
    
    delay.tv_sec = 0;
    delay.tv_usec = 0;

    verbosity = 0;
        
    uuid_clear(uuid);
    
    disable_pidfile = false;
}

bool config_entry_test_set_x_y_strict(const TiXmlElement * element, double & x, double & y){
    {
        const char * txt_x = element->Attribute("x");
        if (txt_x != NULL) { 
            char * endptr = NULL;   
            x = strtod(txt_x, &endptr);
            if (endptr != txt_x+strlen(txt_x)){ return false; }
        }
    }
    {
        const char * txt_y = element->Attribute("y");
        if (txt_y != NULL) { 
            char * endptr = NULL;
            y = strtod(txt_y, &endptr);
            if (endptr != txt_y+strlen(txt_y)){ return false; }
        }
    }
    return true;
}
static int32_t generate_instance_id(const std::string & source){
    srand(time(NULL));

    // deterministically determine the instance id for instance check
    int32_t hash = 0; //rand()%(0xffff); // otherwise the instance would get too long

    int i = 0;
    std::string::const_iterator end = source.end();
    for (std::string::const_iterator c = source.begin(); c != end; c++){
        hash += i*(*c);
        i++;
    }

    return hash;
}
std::string get_app_id(std::string path){

    // default hostname in case of gethostname failure
    const char * defaultHostname = "localhost";

    // RFC 2181 stands clearly that this shall never be longer than 255 octets
    const int hostnameBufferLength = 256;
    char hostnameBuffer[hostnameBufferLength];
    bzero(hostnameBuffer, hostnameBufferLength); // make sure it's an empty space

    errno = 0;
    // error detection
    if (gethostname(hostnameBuffer, hostnameBufferLength) == -1){
        switch (errno){
            // following means that the hostname has been truncated, correct possible missing terminating null
            case EINVAL:
            case ENAMETOOLONG: {
                hostnameBuffer[hostnameBufferLength - 1] = 0;
                std::cerr << "hostname has been truncated (errno: " << errno << "), using '" << hostnameBuffer << "' for hostname detection" << std::endl;
                break;
            }
            // otherwise use the default hostname
            case EPERM:
            case EFAULT: {
                strcpy(hostnameBuffer, defaultHostname);
                std::cerr << "gethostname has failed (errno: " << errno << "), using " << hostnameBuffer << std::endl;
                break;
            }
            
        }
    }

    // extract the short name
    {
        char * dot = strchrnul(hostnameBuffer, '.'); // '.' is host/domain separator
        *dot = 0; // not interested in the domain stuff, just hostname
    }

    // std::string typecasting
    std::string retval("mwtouch(");
    retval.append(hostnameBuffer).append(", ").append(path).append(")");

    // should now consist of "mwtouch(hostname, device path)"
    return retval;
}
int32_t get_ipv4_addr(){

    struct ifaddrs * interfaces = NULL;
    getifaddrs(&interfaces);
    int32_t address = 0;

    bool found = false;

    // search for first IPv4 address of first non-loopback interface if found, loopback otherwise
    for (struct ifaddrs * current = interfaces; (current != NULL) && (!found); current = current->ifa_next){

        // unfortunately, current specs of TUIO 2.0 consideres only the IPv4
        if ((current->ifa_addr != NULL) && (current->ifa_addr->sa_family==AF_INET)){

            // until any non-loopback interface is found, consider loopback
            bool isLoopback = strncmp(current->ifa_name, "lo", 2) == 0;
            if (((address == 0) && (isLoopback)) || (!isLoopback)) {
                address = ((sockaddr_in *)(current->ifa_addr))->sin_addr.s_addr;
            }

            // non-loopback interface was found, use it
            if (!isLoopback){
                found = true;
            }

        }

    }

    // better stay on the safe-side of SIGSEGV
    freeifaddrs(interfaces);
    interfaces = NULL;

    // convert from network byte order
    address = ntohl(address);

    // for some reason there's no network interface active, so at least pretend to run on localhost
//    if (address == 0){ address = 0x7F000001; }

    return address;
}
bool get_pidfile_name(node_config & config){
    const char * tmp = tmpnam(NULL);
    if (tmp == NULL){
        std::cerr << "Failed to get tmp directory path!" << std::endl;
        return false;
    }
    
    config.pidfile_name = tmp;
    size_t bname_pos = config.pidfile_name.find_last_of("/");
    if (bname_pos != std::string::npos){
        config.pidfile_name.erase(bname_pos);
    }
    
    config.pidfile_name.append("/mwtouch");
    
    bname_pos = config.device_path.find_last_of("/");
    if (bname_pos != std::string::npos){
        ++bname_pos;
        if (bname_pos >= config.device_path.length()){
            return false;
        }
            
        config.pidfile_name.append("_");
        config.pidfile_name.append(config.device_path.substr(bname_pos));
    }
    config.pidfile_name.append(".pid");
    
    return true;
}

bool load_config_file(node_config & config){
    if ((config.config_path.compare("-") == 0) && (config.device_path.compare("-"))){
        std::cerr << "Cannot use stdin as both input for config and stored data. Choose one and choose wisely." << std::endl;
        return false;
    }
    
    TiXmlDocument xml_config;
    
    if (config.config_path.compare("-") == 0){
        xml_config.LoadFile(stdin, TIXML_ENCODING_UNKNOWN);
    } else {
        xml_config.LoadFile(config.config_path);
    }
    if (xml_config.Error()){
        std::cerr << "Unable to load config file, given error: " << xml_config.ErrorDesc() << std::endl;
    }
    
    
    const TiXmlElement * e_muse_config = xml_config.RootElement();
    assert(e_muse_config != NULL);
    
    const TiXmlElement * e_wrapper = e_muse_config->FirstChildElement("wrapper");
    bool found_wrapper = false;
    
    while ((!found_wrapper) && (e_wrapper != NULL)){
        node_config tmp_config = config;
        
        // forward to make the gcc shut up
        const TiXmlElement * e_wrapper_config = NULL;
        
        // this one is unfit to rule the gallaxy
        const char * wrapper_type = e_wrapper->Attribute("name");
        if (strcmp(wrapper_type, "mwtouch") != 0) { goto unfit; }
        
        e_wrapper_config =  e_wrapper->FirstChildElement("config");
        if (e_wrapper_config == NULL){ goto unfit; }
        
        { // load device
            const TiXmlElement * e_device = e_wrapper_config->FirstChildElement("device");
            // override-sensitive setting
            if ((e_device != NULL) && (tmp_config.device_path.empty())){ 
                tmp_config.device_path = e_device->GetText(); 
            }
        }
        
        { // load target
            const TiXmlElement * e_target = e_wrapper_config->FirstChildElement("target");
            // override-sensitive setting
            if ((e_target != NULL) && (tmp_config.target_addr.empty())){ 
                tmp_config.target_addr = e_target->GetText(); 
            }
        }
        
        { // find corresponding sensor
            const TiXmlElement * e_sensor = e_wrapper_config->FirstChildElement("sensor");
            bool sensor_found = false;
            
            while ((!sensor_found) && (e_sensor != NULL)){
                
                const char * sensor_uuid = e_sensor->Attribute("uuid");
                { // check sensor uuid & setup dtuio sensor messages
                    if (sensor_uuid == NULL){ goto sensor_unfit; }

                    uuid_t tmp_uuid;
                    uuid_t tmp_empty_uuid;
                    uuid_clear(tmp_empty_uuid);

                    if (uuid_parse(sensor_uuid, tmp_uuid) != 0){
                        std::cerr << "Invalid UUID \"" << sensor_uuid << "\"! Skipping entry!" << std::endl;
                        goto sensor_unfit;
                    }
                    
                    if (uuid_compare(tmp_empty_uuid, tmp_config.uuid) == 0){
                        std::cout << "Sensor UUID not specified, using the first one found!" << std::endl;
                        uuid_copy(tmp_config.uuid, tmp_uuid);
                    }
                    if (uuid_compare(tmp_uuid, tmp_config.uuid) != 0) { goto sensor_unfit; }
                    
                    // uuid done, setup default dtuio sensor
                    dtuio::sensor::sensor_properties * sensor_props = new dtuio::sensor::sensor_properties(
                        tmp_config.uuid,
                        dtuio::sensor::sensor_properties::COORDINATE_INTACT,
                        dtuio::sensor::sensor_properties::PURPOSE_EVENT_SOURCE
                    );
                    
                    do { // sensor purpose extraction
                        const char * sensor_purpose = e_sensor->Attribute("purpose");
                        if (sensor_purpose == NULL){ break; }
                        // set from default to real value
                        
                        if (strcmp(sensor_purpose, "source")) {
                            sensor_props->set_sensor_purpose(dtuio::sensor::sensor_properties::PURPOSE_EVENT_SOURCE);
                        } else if (strcmp(sensor_purpose, "observer")) {
                            sensor_props->set_sensor_purpose(dtuio::sensor::sensor_properties::PURPOSE_OBSERVER);
                        } else if (strcmp(sensor_purpose, "tagger")) {
                            sensor_props->set_sensor_purpose(dtuio::sensor::sensor_properties::PURPOSE_TAGGER);
                        } else {
                            std::cerr << "Unrecognized sensor purpose \"" << sensor_purpose << "\", defaulting to \"source\"!" << std::endl;
                        }
                    } while (false);
                    
                    do { // coordinate translation setup
                        const char * translation_mode = e_sensor->Attribute("coordinate_translation");
                        if (translation_mode == NULL){ break; }
                        // set from default to real value
                        
                        if (strcmp(translation_mode, "setup_once")) {
                            sensor_props->set_coordinate_translation_mode(dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_ONCE);
                        } else if (strcmp(translation_mode, "setup_continuous")) {
                            sensor_props->set_coordinate_translation_mode(dtuio::sensor::sensor_properties::COORDINATE_TRANSLATE_SETUP_CONTINUOUS);
                        } else if (strcmp(translation_mode, "intact")) {
                            sensor_props->set_coordinate_translation_mode(dtuio::sensor::sensor_properties::COORDINATE_INTACT);
                        } else {
                            std::cerr << "Unrecognized coordinate translation mode \"" << translation_mode << "\", defaulting to \"intact\"!" << std::endl;
                        }
                    } while (false);

                    tmp_config.prepared_dtuio.push_back(sensor_props);
                }
                
                { // viewport
                    const TiXmlElement * e_viewport = e_sensor->FirstChildElement("viewport");
                    if (e_viewport != NULL) {
                        const char * width = e_viewport->Attribute("width");
                        if ((width != NULL) && (tmp_config.virtual_sensor_width == DIMMENSION_UNSET)) { 
                            int width_i = strtol(width, NULL, 10);
                            if (width_i <= 0){
                                std::cerr << "Sensor width must be greater than 0!" << std::endl;
                                return false;
                            }
                            tmp_config.virtual_sensor_width = width_i;
                        }
                        
                        const char * height = e_viewport->Attribute("height");
                        if ((height != NULL) && (tmp_config.virtual_sensor_height == DIMMENSION_UNSET)) { 
                            int height_i = strtol(height, NULL, 10);
                            tmp_config.virtual_sensor_height = height_i;
                            if (height_i <= 0){
                                std::cerr << "Sensor height must be greater than 0!" << std::endl;
                                return false;
                            }
                        }
                    }
                }

                { // active quadrangle
                    const TiXmlElement * e_active_quadrangle = e_sensor->FirstChildElement("active_quadrangle");
                    if (e_active_quadrangle != NULL){
                    
                        const char * corner_names[] = {"top_left", "top_right", "bottom_left", "bottom_right", NULL};
                        geometry_point * corners[] = {&tmp_config.top_left, &tmp_config.top_right, &tmp_config.bottom_left, &tmp_config.bottom_right, NULL};

                        for (int i = 0; i < 4; i++){
                            const TiXmlElement * element = e_active_quadrangle->FirstChildElement(corner_names[i]);
                            if (element == NULL) { 
                                std::cerr << "Missing entry \"" << corner_names[i] << "\""
                                    << " in config file - sensor/active_quadrangle has to contain"
                                    << " all of top_left, top_right, bottom_left, bottom_right corners!" << std::endl;
                                return false;
                            
                            }
                            if (!config_entry_test_set_x_y_strict(element, corners[i]->x, corners[i]->y)) {
                                std::cerr << "At least one coordinate for entry \"" << corner_names[i] << "\""
                                    << " is either invalid or missing!" << std::endl;
                                return false;
                            }
                        }

                    } else if (
                        (tmp_config.virtual_sensor_height != DIMMENSION_UNSET)
                     && (tmp_config.virtual_sensor_width != DIMMENSION_UNSET)
                    ){
                        tmp_config.top_left = geometry_point(0, 0);
                        tmp_config.top_right = geometry_point(tmp_config.virtual_sensor_width, 0);
                        tmp_config.bottom_left = geometry_point(0, tmp_config.virtual_sensor_height);
                        tmp_config.top_left = geometry_point(tmp_config.virtual_sensor_width, tmp_config.virtual_sensor_height);
                    } else {
                        std::cout << "Either one of virtual sensor dimmensions is left unset, disabling coordinate transformations." << std::endl;
                        tmp_config.disable_transformations = true;
                    }
                }
                
                do { // mappings
                    int mappin_priority = 30;

                    const TiXmlElement * e_mapping = e_sensor->FirstChildElement("mapping");
                    if (e_mapping == NULL){ break; }
                    
                    const TiXmlElement * e_virtual_axis = e_mapping->FirstChildElement("virtual_axis");
                    while (e_virtual_axis != NULL){
                        char  virtual_name[33];
                        memset(virtual_name, 0, 33);
                        strncpy(virtual_name, e_virtual_axis->Attribute("name"), 32);
                        std::transform(virtual_name, virtual_name + 32, virtual_name, toupper);
                        
                        ev_code_t mapped_onto = get_evcode_for_name(virtual_name);
                        
                        bool is_ok = false;
                        switch (mapped_onto){
                            case ABS_X:
                            case ABS_Y:
                            case ABS_Z: { is_ok = true; break; }
                        }

                        const TiXmlElement * e_mapped_axis = e_virtual_axis->FirstChildElement("axis");

                        if (!is_ok){
                            std::cerr << "Mapping onto axis " << virtual_name << " is not supported!" << std::endl;
                            goto ignore_virtual_axis;
                        }
                        
                        while (e_mapped_axis != NULL){
                            char mapped_name[33];
                            memset(mapped_name, 0, 33);
                            strncpy(mapped_name, e_mapped_axis->GetText(), 32);
                            std::transform(mapped_name, mapped_name + 32, mapped_name, toupper);
                            
                            ev_code_t mapped_from = get_evcode_for_name(mapped_name);
                            if (mapped_from == MAPPING_IGNORE_CODE){
                                std::cerr << "Warning: Mapping " << e_mapped_axis->GetText() << " was not recognized!" << std::endl;
                            } else {
                                axis_mapping tmp;
                                
                                tmp.priority = ++mappin_priority;
                                tmp.code = mapped_onto;
    
                                tmp_config.axes_mappings[mapped_from] = tmp;
                            }
                            
                            e_mapped_axis = e_mapped_axis->NextSiblingElement("axis");
                        }
                    ignore_virtual_axis:
                        e_virtual_axis = e_virtual_axis->NextSiblingElement("virtual_axis");
                    }

                    do {
                        const TiXmlElement * e_ignore = e_mapping->FirstChildElement("ignore");
                        if (e_ignore == NULL){ break; }
                    
                        const TiXmlElement * e_mapped_axis = e_ignore->FirstChildElement("axis");
                        while (e_mapped_axis != NULL){
                            char mapped_name[33];
                            memset(mapped_name, 0, 33);
                            strncpy(mapped_name, e_mapped_axis->GetText(), 32);
                            std::transform(mapped_name, mapped_name + 32, mapped_name, toupper);
                            
                            ev_code_t mapped_from = get_evcode_for_name(mapped_name);
                            if (mapped_from == MAPPING_IGNORE_CODE){
                                std::cerr << "Warning: Mapping " << e_mapped_axis->GetText() << " was not recognized!" << std::endl;
                            } else {
                                axis_mapping tmp;
                                
                                tmp.priority = ++mappin_priority;
                                tmp.code = MAPPING_IGNORE_CODE;
    
                                tmp_config.axes_mappings[mapped_from] = tmp;
                            }
                            
                            e_mapped_axis = e_mapped_axis->NextSiblingElement("axis");
                        }
                    } while (false);
                    
                } while (false);
                
                do { // cehck for group registration
                    const TiXmlElement * e_group = e_sensor->FirstChildElement("group");
                    if (e_group == NULL){ break; }
                    
                    const char * group_uuid = e_group->Attribute("uuid");
                    if (group_uuid == NULL){ 
                        std::cerr << "Mallformed group (missing uuuid) in sensor \"" << sensor_uuid << "\"! Skipping sensor entry!" << std::cerr;
                        goto sensor_unfit;
                    }

                    uuid_t tmp_uuid;
                    if (uuid_parse(group_uuid, tmp_uuid) != 0){
                        std::cerr << "Invalid UUID \"" << group_uuid << "\"! Skipping entry!" << std::endl;
                        goto sensor_unfit;
                    }

                    // uuid done, setup group for this sensor
                    dtuio::sensor_topology::group_member * group_member = new dtuio::sensor_topology::group_member(
                        tmp_uuid, tmp_config.uuid
                    );
                    
                    tmp_config.prepared_dtuio.push_back(group_member);
                } while (false);
                
                { // cehck for neighbours
                    const TiXmlElement * e_neighbour = e_sensor->FirstChildElement("neighbour");
                    while (e_neighbour != NULL){
                        const char * neighbour_uuid = e_neighbour->Attribute("uuid");
                        const char * neighbour_azimuth = e_neighbour->Attribute("azimuth");
                        const char * neighbour_altitude = e_neighbour->Attribute("altitude");
                        const char * neighbour_distance = e_neighbour->Attribute("distance");

                        double azimuth = 0;
                        double altitude = 0;
                        double distance = 0;
                        
                        { // azimuth
                            char * endptr = NULL;
                            azimuth = strtod(neighbour_azimuth, &endptr);
                            if (endptr != neighbour_azimuth + strlen(neighbour_azimuth)){
                                std::cerr << "Mallformed azimuth attribute! Skipping sensor entry!" << std::cerr;
                                goto sensor_unfit;
                            }
                        }
                        { // altitude
                            char * endptr = NULL;
                            altitude = strtod(neighbour_altitude, &endptr);
                            if (endptr != neighbour_altitude + strlen(neighbour_altitude)){
                                std::cerr << "Mallformed altitude attribute! Skipping sensor entry!" << std::cerr;
                                goto sensor_unfit;
                            }
                        }
                        { // distance
                            char * endptr = NULL;
                            distance = strtod(neighbour_distance, &endptr);
                            if (endptr != neighbour_distance + strlen(neighbour_distance)){
                                std::cerr << "Mallformed distance attribute! Skipping sensor entry!" << std::cerr;
                                goto sensor_unfit;
                            }
                        }
                        
                        dtuio::sensor_topology::neighbour * neighbour = new dtuio::sensor_topology::neighbour(
                            tmp_config.uuid, azimuth, altitude, distance
                        );
                        tmp_config.prepared_dtuio.push_back(neighbour);

                        if (neighbour_uuid != NULL){ 
                            uuid_t tmp_uuid;
                            if (uuid_parse(neighbour_uuid, tmp_uuid) != 0){
                                std::cerr << "Invalid neighbour UUID \"" << neighbour_uuid << "\"! Skipping entry!" << std::endl;
                                goto sensor_unfit;
                            }
                            neighbour->set_neighbour_uuid(tmp_uuid);
                        }
                        
                        e_neighbour = e_neighbour->NextSiblingElement("neighbour");
                    }
                }
                
//            sensor_fit:
                sensor_found = true;
                continue;
                
            sensor_unfit:
                e_sensor = e_sensor->NextSiblingElement("sensor");
                continue;
            }
            
            if (!sensor_found){ goto unfit; }
        }
        
//        fit:
            config = tmp_config;
            found_wrapper = true;
            continue;

        unfit:
            e_wrapper = e_wrapper->NextSiblingElement("wrapper");
            continue;
    }
    
    if (!found_wrapper){
        std::cerr << "No valid wrapper configuration has been found!" << std::endl;
    }

    return found_wrapper;
}
bool complete_config(node_config & config){
    // complete config
    config.app_name = get_app_id(config.device_path);
    config.local_ip = get_ipv4_addr();
    config.instance = generate_instance_id(config.app_name);
    
    // setup dtuio viewport
    if ((config.virtual_sensor_width != DIMMENSION_UNSET) && (config.virtual_sensor_width != DIMMENSION_UNSET)){
        dtuio::sensor::viewport * vpt = new dtuio::sensor::viewport(config.uuid, config.virtual_sensor_width, config.virtual_sensor_height, 0);
        config.prepared_dtuio.push_back(vpt);
    }
    
    // extract target port from addr
    if (config.target_addr.find(':') == std::string::npos){
        std::cerr << "Port not found in '" << config.target_addr << "', using default (3333)" << std::endl;
        config.target_addr.append(":3333");
    }

    get_pidfile_name(config);
    
    return true;
}

static void imprint_axis(TiXmlElement & axis, const axis_mapping_map & mappings, ev_code_t code){
    axis_mapping_map::const_iterator end = mappings.end();
    axis_mapping_map::const_iterator beg = mappings.begin();

    for (axis_mapping_map::const_iterator i = beg; i != end; i++){
        if (i->second.code == code){
            std::string axis_name = get_abs_ev_name_short(i->first);
            std::transform(axis_name.begin(), axis_name.end(), axis_name.begin(), tolower);
            TiXmlElement ax("axis");
            ax.InsertEndChild(TiXmlText(axis_name));
            axis.InsertEndChild(ax);
        }
    }
}

void write_config(const node_config & config, int ofd){
    // this should get a little bit less complicated
    
    TiXmlDocument config_doc;
    config_doc.InsertEndChild(TiXmlDeclaration("1.0", "", ""));

    std::cout << "Calibration done, dumping config file..." << std::endl << std::endl;

    TiXmlElement * e_sensor = NULL;
    {
        // root
        TiXmlElement root_node_tmp("muse_config");
        config_doc.InsertEndChild(root_node_tmp);
        TiXmlElement * e_muse_config = config_doc.RootElement();
        
        // wrapper
        TiXmlElement wrapper_tmp("wrapper");
        wrapper_tmp.SetAttribute("name", "mwtouch");
        e_muse_config->InsertEndChild(wrapper_tmp);
        TiXmlElement * e_wrapper = e_muse_config->FirstChildElement("wrapper");
        
        // config
        TiXmlElement config_tmp("config");
        e_wrapper->InsertEndChild(config_tmp);
        TiXmlElement * e_config = e_wrapper->FirstChildElement("config");
        
        // wrapper/target
        e_config->InsertEndChild(TiXmlComment("Where to send the data"));
        TiXmlElement target_tmp("target");
        TiXmlNode * n_target = e_config->InsertEndChild(target_tmp);
        n_target->InsertEndChild(TiXmlText("localhost:3333"));
        
        // wrapper/target
        e_config->InsertEndChild(TiXmlComment("Which device this wrapper operates on"));
        TiXmlElement device_tmp("device");
        TiXmlNode * n_device = e_config->InsertEndChild(device_tmp);
        n_device->InsertEndChild(TiXmlText(config.device_path));
        
        // wrapper/sensor
        e_config->InsertEndChild(TiXmlComment(
            "\nThe very sensor & wrapper configuration; \n"
            "coordinate_translation: one of \"setup_once\", \"setup_continuous\" and \"intact\";\n"
            "purpose: one of \"source\", \"observer\" and \"tagger\"\n"
        ));
        TiXmlElement sensor_tmp("sensor");
        e_config->InsertEndChild(sensor_tmp);
        e_sensor = e_config->FirstChildElement("sensor");
        
        char buffer[40];
        uuid_unparse(config.uuid, buffer);
        
        e_sensor->SetAttribute("uuid", buffer);
        e_sensor->SetAttribute("coordinate_translation", "setup_once");
        e_sensor->SetAttribute("purpose", "source");
        
    }
    
    {
        e_sensor->InsertEndChild(TiXmlComment("The virtual rectangle of this virtual sensor"));
        TiXmlElement viewport_tmp("viewport");
        viewport_tmp.SetAttribute("width", config.virtual_sensor_width);
        viewport_tmp.SetAttribute("height", config.virtual_sensor_height);
        e_sensor->InsertEndChild(viewport_tmp);
    }

    {
        e_sensor->InsertEndChild(TiXmlComment("The result of mapping"));
        TiXmlElement quadrangle_tmp("active_quadrangle");
        
        TiXmlElement top_left("top_left");
        top_left.SetAttribute("x", config.top_left.x);
        top_left.SetAttribute("y", config.top_left.y);
        quadrangle_tmp.InsertEndChild(top_left);
        
        TiXmlElement top_right("top_right");
        top_right.SetAttribute("x", config.top_right.x);
        top_right.SetAttribute("y", config.top_right.y);
        quadrangle_tmp.InsertEndChild(top_right);

        TiXmlElement bottom_right("bottom_right");
        bottom_right.SetAttribute("x", config.bottom_right.x);
        bottom_right.SetAttribute("y", config.bottom_right.y);
        quadrangle_tmp.InsertEndChild(bottom_right);

        TiXmlElement bottom_left("bottom_left");
        bottom_left.SetAttribute("x", config.bottom_left.x);
        bottom_left.SetAttribute("y", config.bottom_left.y);
        quadrangle_tmp.InsertEndChild(bottom_left);

        e_sensor->InsertEndChild(quadrangle_tmp);
    }
    
    { // write axis mappings
        TiXmlElement e_mappings("mapping");
        
        e_sensor->InsertEndChild(TiXmlComment("If specified, the given axes will override the default mapping"));
        { // x
            TiXmlElement axis_x("virtual_axis");
            axis_x.SetAttribute("name", "x");
            imprint_axis(axis_x, config.axes_mappings, ABS_X);
            e_mappings.InsertEndChild(axis_x);
        }
    
        { // y
            TiXmlElement axis_y("virtual_axis");
            axis_y.SetAttribute("name", "y");
            imprint_axis(axis_y, config.axes_mappings, ABS_Y);
            e_mappings.InsertEndChild(axis_y);
        }

        { // ignore
            e_mappings.InsertEndChild(TiXmlComment("Following input axes shall be completly ignored"));
            TiXmlElement ignore("ignore");
            imprint_axis(ignore, config.axes_mappings, MAPPING_IGNORE_CODE);
            e_mappings.InsertEndChild(ignore);
        }
        
        e_sensor->InsertEndChild(e_mappings);
    }
    
    { // dtuio examples
        { // group example
            uuid_t group_uuid;
            uuid_clear(group_uuid);
            uuid_generate(group_uuid);
//                std::cerr << "Unable to produce safe UUID for group." << std::endl;
            char buffer[40];
            uuid_unparse(config.uuid, buffer);

            e_sensor->InsertEndChild(TiXmlComment("An example of group membership tag"));
            e_sensor->InsertEndChild(TiXmlComment(std::string("<group uuid=\"").append(buffer).append("\" />").c_str()));
        }
        { // neighbour example
            uuid_t neighbour_uuid;
            uuid_clear(neighbour_uuid);
            uuid_generate(neighbour_uuid);
//                std::cerr << "Unable to produce safe UUID for neighbour." << std::endl;
            char buffer[40];
            uuid_unparse(config.uuid, buffer);

            e_sensor->InsertEndChild(TiXmlComment("An example of neighbour report"));
            e_sensor->InsertEndChild(TiXmlComment(
                std::string("<neighbour azimuth=\"-1.570796327\" altitude=\"0\""
                " distance=\"1080\" uuid=\"").append(buffer).append("\" />").c_str()
            ));
        }
    }
    
    FILE * output = fdopen(ofd, "w");
    config_doc.SaveFile(output);
    fflush(output);
}

