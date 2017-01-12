/**
 * \file      nodeconfig.cpp
 * \brief     Provides the struct that holds the wrapper configuration
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-03-17 19:14 UTC+2
 * \copyright BSD
 */

#include <errno.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <endian.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <string>
#include <algorithm>
#include <tinyxml.h>
#include <kerat/kerat.hpp>
#include <dtuio/dtuio.hpp>

#include "nodeconfig.hpp"
#include "kinect_device.hpp"

//#define VERBOSITY_LEVEL_DUMP (1 << 0)
//#define DIMMENSION_UNSET ((uint16_t)-1)

// since the singleton core has to be somewhere anyway
node_config * node_config::m_instance = NULL;

kinect_device_config::kinect_device_config()
    :kinect_camera_resolution(FREENECT_RESOLUTION_DUMMY),
    kinect_depth_resolution(FREENECT_RESOLUTION_DUMMY),
    kinect_camera_format(FREENECT_VIDEO_DUMMY),
    kinect_depth_format(FREENECT_DEPTH_DUMMY),
    transform_input_image(false),
    kinect_context(NULL)
{ ; }
    
void kinect_device_config::set_defaults() {
    kinect_camera_resolution = FREENECT_RESOLUTION_MEDIUM;
    kinect_depth_resolution = FREENECT_RESOLUTION_HIGH;
    kinect_camera_format = FREENECT_VIDEO_YUV_RGB;
    kinect_depth_format = FREENECT_DEPTH_11BIT;
    
    reset_transformations();
}

void kinect_device_config::reset_transformations(){

    distortion_matrix_ir  = cv::Mat::zeros(cv::Size(5, 1), CV_32FC1);
    distortion_matrix_rgb = distortion_matrix_ir.clone();

    cv::Mat tmp = cv::Mat::ones(cv::Size(3, 3), CV_32FC1).diag(0);
    camera_matrix_ir  = tmp.clone();
//        camera_matrix_ir.at<float>(0, 2) = 1; // this should actualy be width/2
//        camera_matrix_ir.at<float>(0, 1) = 1; // this should actualy be height/2
    camera_matrix_rgb = camera_matrix_ir.clone();

    tmp = cv::Mat::ones(3, 3, CV_32FC1).diag(0);
    stereo_matrix_r = tmp;
    stereo_matrix_t = cv::Mat::ones(3, 1, CV_32FC1);

    transform_input_image = false;
    
}
    
kinect_device_config & kinect_device_config::operator=(const kinect_device_config & config){
    if (&config == this) { return *this; }

    kinect_camera_resolution = config.kinect_camera_resolution;
    kinect_depth_resolution  = config.kinect_depth_resolution;
    kinect_camera_format     = config.kinect_camera_format;
    kinect_depth_format      = config.kinect_depth_format;

    stereo_matrix_r = config.stereo_matrix_r.clone();
    stereo_matrix_t = config.stereo_matrix_t.clone();
    camera_matrix_rgb = config.camera_matrix_rgb.clone();
    camera_matrix_ir  = config.camera_matrix_ir.clone();
    distortion_matrix_rgb = config.distortion_matrix_rgb.clone();
    distortion_matrix_ir  = config.distortion_matrix_ir.clone();

    kinect_context = config.kinect_context;
    transform_input_image = config.transform_input_image;
    device_serial = config.device_serial;

    return *this;
}
    
sensor_config::sensor_config()
    :    virtual_sensor_width(-1), virtual_sensor_height(-1),
    depth_threshold_min(-1), depth_threshold_max(-1),
    blob_area_min(-1), blob_area_max(-1)
{
    uuid_clear(uuid);
}
    
void sensor_config::set_defaults() {
    virtual_sensor_width = 1920;
    virtual_sensor_height = 1080;

    depth_threshold_max = 200*4;
    depth_threshold_min = 200;

    blob_area_min = 400;
    blob_area_max = 40000;
}

sensor_config & sensor_config::operator=(const sensor_config & config){
    if (&config == this) { return *this; }

    uuid_copy(uuid, config.uuid);
    virtual_sensor_width     = config.virtual_sensor_width;
    virtual_sensor_height    = config.virtual_sensor_height;

    depth_threshold_min = config.depth_threshold_min;
    depth_threshold_max = config.depth_threshold_max;

    blob_area_min = config.blob_area_min;
    blob_area_max = config.blob_area_max;

    return *this;
}
    

wrapper_core_config::wrapper_core_config():local_ip(0), instance(0){ ; }

thread_com::thread_com(updatable_config * cfg)
    :argc(0), argv(NULL), 
    gui_thread(0), 
    config(cfg), gui_instance(NULL),
    keep_running(true)
{ ; }

updatable_config::updatable_config():force_update(false){ ; }

updatable_config::~updatable_config(){ ; }

node_config::node_config()
    :device(NULL),
    verbosity(0), disable_pidfile(false),
    ui(UI_UNSET), thread_com_channel(this)
{ ; }

node_config::~node_config(){
    if (device != NULL){
        delete device;
        device = NULL;
    }
    
    if (kinect_context != NULL) {
        freenect_shutdown(kinect_context);
        kinect_context = NULL;
    }
}

node_config * node_config::get_instance(){ 
    if (m_instance == NULL){ 
        m_instance = new node_config; 
    } 
    return m_instance; 
}
void node_config::destroy(){
    if (m_instance != NULL){ 
        delete m_instance;
        m_instance = NULL;
    } 
}


void node_config::set_defaults(){
    ui = UI_AUTODETECT;
    operation_mode = MODE_WRAPPER;
    verbosity = 0;
    disable_pidfile = false;
}

bool load_config_file(node_config & config){
    TiXmlDocument xml_config;
    xml_config.LoadFile(config.config_path);
    
    const TiXmlElement * e_muse_config = xml_config.RootElement();
    assert(e_muse_config != NULL);
    
    const TiXmlElement * e_wrapper = e_muse_config->FirstChildElement("wrapper");
    bool found_wrapper = false;
    
    while ((!found_wrapper) && (e_wrapper != NULL)){
        std::string tc_device_serial = config.device_serial;
        std::string tc_target_addr   = config.target_addr;
        node_config::ui_t tc_ui      = config.ui;
        dtuio::uuid_t tc_uuid; uuid_copy(tc_uuid, config.uuid);
        int16_t tc_virtual_sensor_width = config.virtual_sensor_width;
        int16_t tc_virtual_sensor_height = config.virtual_sensor_height;
        wrapper_core_config::dtuio_list tc_prepared_dtuio = config.prepared_dtuio;
        int tc_blob_area_min = config.blob_area_min;
        int tc_blob_area_max = config.blob_area_max;
        int tc_depth_threshold_min = config.depth_threshold_min;
        int tc_depth_threshold_max = config.depth_threshold_max;
        std::string tc_kinect_calib_string = config.kinect_calib_string;

        freenect_resolution tc_video_resolution = config.kinect_camera_resolution;
        freenect_video_format tc_video_format = config.kinect_camera_format;
        
        freenect_resolution tc_depth_resolution = config.kinect_depth_resolution;
        freenect_depth_format tc_depth_format = config.kinect_depth_format;
        
        // forward to make the gcc shut up
        const TiXmlElement * e_wrapper_config = NULL;
        
        // this one is unfit to rule the gallaxy
        const char * wrapper_type = e_wrapper->Attribute("name");
        if (strcmp(wrapper_type, "mwkinect") != 0) { goto unfit; }
        
        e_wrapper_config =  e_wrapper->FirstChildElement("config");
        if (e_wrapper_config == NULL){ goto unfit; }
        
        { // load device
            if (tc_device_serial.empty()){
                const TiXmlElement * e_device = e_wrapper_config->FirstChildElement("device");
                // override-sensitive setting
                if (e_device != NULL){ 
                    const char * devsn = e_device->GetText();
                    tc_device_serial = devsn;
                } else {
                    std::cerr << "Device id is required, either through the config file or command line argument!" << std::endl;
                    return false;
                }
            }
        }

        { // load target
            const TiXmlElement * e_target = e_wrapper_config->FirstChildElement("target");
            // override-sensitive setting
            if ((e_target != NULL) && (tc_target_addr.empty())){ 
                tc_target_addr = e_target->GetText(); 
            }
        }
        
        { // load ui
            const TiXmlElement * e_ui = e_wrapper_config->FirstChildElement("ui");
            // override-sensitive setting
            if ((e_ui != NULL) && (tc_target_addr.empty())){ 
                const char * input = e_ui->GetText(); 
                if (input == NULL){
                    std::cerr << "Empty UI tag in config file, switching to autodetection!" << std::endl;
                    tc_ui = node_config::UI_AUTODETECT;
                } else if (strcmp(input, "auto") == 0){
                    tc_ui = node_config::UI_AUTODETECT;
                } else if (strcmp(input, "gtk") == 0){
                    tc_ui = node_config::UI_GTK;
                } else if (strcmp(input, "console") == 0){
                    tc_ui = node_config::UI_CONSOLE;
                } else {
                    std::cerr << "Unsupported ui type \"" << input << "\"! Falling back to console!" << std::endl;
                    tc_ui = node_config::UI_CONSOLE;
                }
            }
        }
        
        { // find corresponding sensor
            const TiXmlElement * e_sensor = e_wrapper_config->FirstChildElement("sensor");
            bool sensor_found = false;
            
            while ((!sensor_found) && (e_sensor != NULL)){
                
                const char * sensor_uuid = e_sensor->Attribute("uuid");
                if (sensor_uuid == NULL){ goto sensor_unfit; }


                { // process sensor uuid
                    uuid_t tmp_uuid;
                    uuid_t tmp_empty_uuid;
                    uuid_clear(tmp_empty_uuid);

                    if (uuid_parse(sensor_uuid, tmp_uuid) != 0){
                        std::cerr << "Invalid UUID \"" << sensor_uuid << "\"! Skipping entry!" << std::endl;
                        goto sensor_unfit;
                    }
                    
                    if (uuid_compare(tmp_empty_uuid, tc_uuid) == 0){
                        std::cout << "Sensor UUID not specified, using the first one found!" << std::endl;
                        uuid_copy(tc_uuid, tmp_uuid);
                    }
                    
                    if (uuid_compare(tmp_uuid, tc_uuid) != 0) { goto sensor_unfit; }
                }
                
                { // store viewport
                    const TiXmlElement * e_viewport = e_sensor->FirstChildElement("viewport");
                    if (e_viewport != NULL) {
                        const char * width = e_viewport->Attribute("width");
                        if ((width != NULL) && (tc_virtual_sensor_width == -1)) { 
                            int width_i = strtol(width, NULL, 10);
                            if (width_i <= 0){
                                std::cerr << "Sensor width must be greater than 0!" << std::endl;
                                return false;
                            }
                            tc_virtual_sensor_width = width_i;
                        }
                        
                        const char * height = e_viewport->Attribute("height");
                        if ((height != NULL) && (tc_virtual_sensor_height == -1)) { 
                            int height_i = strtol(height, NULL, 10);
                            tc_virtual_sensor_height = height_i;
                            if (height_i <= 0){
                                std::cerr << "Sensor height must be greater than 0!" << std::endl;
                                return false;
                            }
                        }
                    }
                }
                
                do { // cehck for group registration
                    const TiXmlElement * e_group = e_sensor->FirstChildElement("group");
                    if (e_group == NULL){ break; }
                    
                    const char * group_uuid = e_sensor->Attribute("uuid");
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
                        tmp_uuid, tc_uuid
                    );
                    
                    tc_prepared_dtuio.push_back(group_member);
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
                            tc_uuid, azimuth, altitude, distance
                        );
                        tc_prepared_dtuio.push_back(neighbour);

                        if (neighbour_uuid != NULL){ 
                            uuid_t tmp_uuid;
                            if (uuid_parse(neighbour_uuid, tmp_uuid) != 0){
                                std::cerr << "Invalid neighbour UUID \"" << neighbour_uuid << "\"! Skipping entry!" << std::endl;
                                goto sensor_unfit;
                            }
                            neighbour->set_uuid(tmp_uuid);
                        }
                        
                        e_neighbour = e_neighbour->NextSiblingElement("neighbour");
                    }
                }
                

                { // store blob_area
                    const TiXmlElement * e_blob_area = e_sensor->FirstChildElement("blob_area");
                    if (e_blob_area != NULL) {
                        const char * min = e_blob_area->Attribute("min");
                        if ((min != NULL) && (tc_blob_area_min == -1)) { 
                            int min_i = strtol(min, NULL, 10);
                            if (min_i <= 0){
                                std::cerr << "Sensor min must be greater than 0!" << std::endl;
                                return false;
                            }
                            tc_blob_area_min = min_i;
                        }
                        
                        const char * max = e_blob_area->Attribute("max");
                        if ((max != NULL) && (tc_blob_area_max == -1)) { 
                            int max_i = strtol(max, NULL, 10);
                            tc_blob_area_max = max_i;
                            if (max_i <= 0){
                                std::cerr << "Sensor max must be greater than 0!" << std::endl;
                                return false;
                            }
                        }
                    }
                }

                { // store depth
                    const TiXmlElement * e_depth = e_sensor->FirstChildElement("depth");
                    if (e_depth != NULL) {
                        const char * min = e_depth->Attribute("min");
                        if ((min != NULL) && (tc_depth_threshold_min == -1)) { 
                            int min_i = strtol(min, NULL, 10);
                            if (min_i <= 0){
                                std::cerr << "Active area depth must be greater than 0!" << std::endl;
                                return false;
                            }
                            tc_depth_threshold_min = min_i;
                        }
                        
                        const char * max = e_depth->Attribute("max");
                        if ((max != NULL) && (tc_depth_threshold_max == -1)) { 
                            int max_i = strtol(max, NULL, 10);
                            tc_depth_threshold_max = max_i;
                            if (max_i >= (1 << 13)) {
                                std::cerr << "Active area depth must not be more than 2^13!" << std::endl;
                                return false;
                            }
                        }
                    }
                }
                
                { // kinect video resolution & mode
                    // check whether set, commandline takes precedence
                    if (tc_video_format == FREENECT_VIDEO_DUMMY) {
                        const TiXmlElement * e_video_mode = e_sensor->FirstChildElement("video_mode");
                        if (e_video_mode != NULL) {
                            if (!nc_parse_video_mode(e_video_mode->GetText(), tc_video_format, tc_video_resolution)){
                                std::cerr << "Video mode \"" << e_video_mode->GetText() << "\" not found!" << std::endl;
                                goto sensor_unfit;
                            }
                        } else {
                            std::cerr << "Video mode has to be set!" << std::endl;
                            goto sensor_unfit;
                        }
                    }
                }
                
                { // kinect depth resolution & mode
                    // check whether set, commandline takes precedence
                    if (tc_depth_format == FREENECT_DEPTH_DUMMY) {
                        const TiXmlElement * e_depth_mode = e_sensor->FirstChildElement("depth_mode");
                        if (e_depth_mode != NULL) {
                            if (!nc_parse_depth_mode(e_depth_mode->GetText(), tc_depth_format, tc_depth_resolution)){
                                std::cerr << "Depth mode \"" << e_depth_mode->GetText() << "\" not found!" << std::endl;
                                goto sensor_unfit;
                            }
                        } else {
                            std::cerr << "Depth mode has to be set!" << std::endl;
                            goto sensor_unfit;
                        }
                    }
                }
                
                { // calib data
                    const TiXmlElement * e_calibration = e_sensor->FirstChildElement("kinect_calibration");
                    if (e_calibration != NULL) {
                        const TiXmlElement * e_cv_storage = e_calibration->FirstChildElement("opencv_storage");
                        if (e_cv_storage != NULL){
                            TiXmlPrinter t_printer;
                            e_cv_storage->Accept(&t_printer);
                            tc_kinect_calib_string = t_printer.CStr();
                        }
                    }
                }

            //sensor_fit:
                sensor_found = true;
                continue;
                
            sensor_unfit:
                e_sensor = e_sensor->NextSiblingElement("sensor");
                continue;
            }
            
            if (!sensor_found){ goto unfit; }
        }
        
        //fit:
            config.device_serial = tc_device_serial;
            config.target_addr   = tc_target_addr;
            config.ui = tc_ui;
            uuid_copy(config.uuid, tc_uuid);
            config.virtual_sensor_width  = tc_virtual_sensor_width;
            config.virtual_sensor_height = tc_virtual_sensor_height;
            config.prepared_dtuio = tc_prepared_dtuio;
            config.blob_area_min  = tc_blob_area_min;
            config.blob_area_max  = tc_blob_area_max;
            config.depth_threshold_min = tc_depth_threshold_min;
            config.depth_threshold_max = tc_depth_threshold_max;
            config.kinect_calib_string = tc_kinect_calib_string;
            
            config.kinect_camera_format = tc_video_format;
            config.kinect_camera_resolution = tc_video_resolution;
            config.kinect_depth_format = tc_depth_format;
            config.kinect_depth_resolution = tc_depth_resolution;
            
            found_wrapper = true;
            continue;

        unfit:
            e_wrapper = e_wrapper->NextSiblingElement("wrapper");
            continue;
    }

    return found_wrapper;
}
bool save_config_file(std::ostream & output, const node_config & config){
    TiXmlDocument xml_config;
    
    xml_config.InsertEndChild(TiXmlDeclaration("1.0", "", ""));

    TiXmlElement * e_sensor = NULL;
        
        // root
        TiXmlElement root_node_tmp("muse_config");
        xml_config.InsertEndChild(root_node_tmp);
        TiXmlElement * e_muse_config = xml_config.RootElement();
        
        // wrapper
        TiXmlElement wrapper_tmp("wrapper");
        wrapper_tmp.SetAttribute("name", "mwkinect");
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
        n_target->InsertEndChild(TiXmlText(config.target_addr));
        
        // wrapper/target
        e_config->InsertEndChild(TiXmlComment("Serial number of device this wrapper operates on"));
        TiXmlElement device_tmp("device");
        TiXmlNode * n_device = e_config->InsertEndChild(device_tmp);
        n_device->InsertEndChild(TiXmlText(config.device_serial));
        
        TiXmlElement sensor_tmp("sensor");
        e_config->InsertEndChild(sensor_tmp);
        e_sensor = e_config->FirstChildElement("sensor");

        { // ui
            e_config->InsertEndChild(TiXmlComment("Which pseudoui to use - options: auto, gtk, console"));
            TiXmlElement ui_tmp("ui");
            TiXmlNode * n_ui = e_config->InsertEndChild(ui_tmp);
            std::string ui_text = "auto";
            
            switch (config.ui){
                case node_config::UI_GTK: { ui_text = "gtk"; break; }
                case node_config::UI_AUTODETECT:
                default: { ui_text = "auto"; break; }
            }
            n_ui->InsertEndChild(TiXmlText(ui_text));
        }
        
        // sensor uuid
        char buffer[40];
        uuid_unparse(config.uuid, buffer);
        e_sensor->SetAttribute("uuid", buffer);

        { // sensor viewport
            TiXmlElement tmp_viewport("viewport");

            std::stringstream sx;
            sx << config.virtual_sensor_width;
            std::string tmp; sx >> tmp;
            tmp_viewport.SetAttribute("width", tmp);
            
            sx.clear();
            sx << config.virtual_sensor_height;
            sx >> tmp;
            tmp_viewport.SetAttribute("height", tmp);

            e_sensor->InsertEndChild(tmp_viewport);
        }
        
        { // sensor blob_area
            TiXmlElement tmp_blob_area("blob_area");

            std::stringstream sx;
            sx << config.blob_area_min;
            std::string tmp; sx >> tmp;
            tmp_blob_area.SetAttribute("min", tmp);
            
            sx.clear();
            sx << config.blob_area_max;
            sx >> tmp;
            tmp_blob_area.SetAttribute("max", tmp);

            e_sensor->InsertEndChild(tmp_blob_area);
        }

        { // sensor blob_area
            TiXmlElement tmp_depth("depth");

            std::stringstream sx;
            sx << config.depth_threshold_min;
            std::string tmp; sx >> tmp;
            tmp_depth.SetAttribute("min", tmp);
            
            sx.clear();
            sx << config.depth_threshold_max;
            sx >> tmp;
            tmp_depth.SetAttribute("max", tmp);

            e_sensor->InsertEndChild(tmp_depth);
        }
        
        // dtuio exports
        {
            for (node_config::dtuio_list::const_iterator i = config.prepared_dtuio.begin(); i != config.prepared_dtuio.end(); ++i){
                const libkerat::kerat_message * msg = *i;
                
                { // group
                    const dtuio::sensor_topology::group_member * grp = dynamic_cast<const dtuio::sensor_topology::group_member *>(msg);
                    if (grp != NULL){
                        if (uuid_compare(config.uuid, grp->get_uuid()) != 0){ continue; }

                        TiXmlElement tmp_group("group");
                        char buffer[50];
                        uuid_unparse(grp->get_group_uuid(), buffer);
                        tmp_group.SetAttribute("uuid", buffer);
                        e_sensor->InsertEndChild(tmp_group);
                        continue;
                    }
                }
                
                { // neighbour
                    const dtuio::sensor_topology::neighbour * nbr = dynamic_cast<const dtuio::sensor_topology::neighbour *>(msg);
                    if (nbr != NULL){
                        if (uuid_compare(config.uuid, nbr->get_uuid()) != 0){ continue; }

                        TiXmlElement tmp_neighbour("neighbour");

                        char buffer[50];
                        uuid_unparse(nbr->get_neighbour_uuid(), buffer);
                        tmp_neighbour.SetAttribute("uuid", buffer);

                        std::stringstream sx; std::string tmp;
                        sx << nbr->get_azimuth();
                        sx >> tmp;
                        tmp_neighbour.SetAttribute("azimuth", tmp);

                        sx.clear();
                        sx << nbr->get_altitude();
                        sx >> tmp;
                        tmp_neighbour.SetAttribute("altitude", tmp);

                        sx.clear();
                        sx << nbr->get_distance();
                        sx >> tmp;
                        tmp_neighbour.SetAttribute("distance", tmp);

                        e_sensor->InsertEndChild(tmp_neighbour);
                        continue;
                    }
                }
            } // for
        } // relevant dtuio messages added
        
        { // calib data
            TiXmlElement tmp_calibration("kinect_calibration");
            TiXmlDocument tmp_doc;
            tmp_doc.Parse(config.kinect_calib_string.c_str(), NULL, TIXML_ENCODING_UTF8);
            tmp_calibration.InsertEndChild(*tmp_doc.RootElement());
            e_sensor->InsertEndChild(tmp_calibration);
        }

    // done with generating, write out
    TiXmlPrinter t_printer;
    xml_config.Accept(&t_printer);
    std::string xml_content = t_printer.CStr();
    output << xml_content;
    
    return true;
}

// complete config helpers
static int32_t get_ipv4_addr(){

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
static std::string get_app_id(const std::string & device_serial){

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

    // string typecasting
    std::stringstream retval_buffer;
    retval_buffer << "mwkinect(" << hostnameBuffer << "/" << device_serial << ")";
    
    std::string retval;
    std::getline(retval_buffer, retval);

    // should now consist of "mwkinect(hostname, device id)"
    return retval;
}
static bool get_pidfile_name(node_config & config){
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
    
    std::stringstream pidfile_name_buffer;
    pidfile_name_buffer << config.pidfile_name;
    config.pidfile_name.clear();
    
    pidfile_name_buffer << "/mwkinect_";
    pidfile_name_buffer << config.device_serial;
    pidfile_name_buffer << ".pid";

    std::getline(pidfile_name_buffer, config.pidfile_name);
    
    return true;
}

bool complete_config(node_config & config){
    // complete config
    //config.thread_com_channel.keep_running = &running;
    config.app_name = get_app_id(config.device_serial);
    config.local_ip = get_ipv4_addr();
    config.instance = generate_instance_id(config.app_name);
    
    // if no gui related option was passed, use autodetect
    if (config.ui == node_config::UI_UNSET){ config.ui = node_config::UI_AUTODETECT; }
    
    if (config.ui == node_config::UI_AUTODETECT){
        config.ui = (getenv("DISPLAY") != NULL)?node_config::UI_GTK:node_config::UI_CONSOLE;
    }

    // extract target port from addr
    if (config.target_addr.find(':') == std::string::npos){
        std::cerr << "Port not found in '" << config.target_addr << "', using default (3333)" << std::endl;
        config.target_addr.append(":3333");
    }

    // OPENCV load dissortions
    config.kinect_calib_string.insert(0, "<?xml version=\"1.0\"?>\n");
    if (!config.kinect_calib_string.empty()){
        cv::FileStorage kinect_calib_data(config.kinect_calib_string, cv::FileStorage::READ | cv::FileStorage::MEMORY);
        if (kinect_calib_data.isOpened()){

            kinect_calib_data["Distortion_IR"] >> config.distortion_matrix_ir;
            kinect_calib_data["Distortion_RGB"] >> config.distortion_matrix_rgb;

            kinect_calib_data["Intrinsics_IR"] >> config.camera_matrix_ir;
            kinect_calib_data["Intrinsics_RGB"] >> config.camera_matrix_rgb;

            kinect_calib_data["kinect_R"] >> config.stereo_matrix_r;
            kinect_calib_data["kinect_T"] >> config.stereo_matrix_t;
            kinect_calib_data.release();
        } else {
            std::cerr << "Unable to load kinect callibration!" << std::endl;
            return false;
        }
    } else {
        std::cout << "No calibration preset, disabling input tranformations..." << std::endl;
        config.transform_input_image = false;
    }

    get_pidfile_name(config);
    
    return true;
}

static bool nc_parser_helper(char * mode, int & width, int & height, int & framerate){
    // case insensitive
    std::transform(mode, mode+strlen(mode), mode, toupper);
    
    char * pt1 = strtok(mode, ":");
    char * pt2 = strtok(NULL, ":");
    char * pt3 = strtok(NULL, ":");
    
    char * cwidth = strtok(pt2, "X");
    char * cheight = strtok(NULL, "X");
    
    if ((pt1 == NULL) || (cwidth == NULL) || (cheight == NULL) || (pt3 == NULL)){ return false; }
    
    char * endptr = NULL;
    width = strtol(cwidth, &endptr, 10);
    if ((endptr != NULL) && (*endptr != 0)){ return false; }

    height = strtol(cheight, &endptr, 10);
    if ((endptr != NULL) && (*endptr != 0)){ return false; }
    
    framerate = strtol(pt3, &endptr, 10);
    if ((endptr != NULL) && (*endptr != 0)){ return false; }
    
    return true;
}

bool nc_parse_video_mode(const char * mode, freenect_video_format & format, freenect_resolution & resolution){
    char buffer[40];
    memset(buffer, 0, 40);
    strncpy(buffer, mode, 39);

    int tmp_width = 0;
    int tmp_height = 0;
    int tmp_framerate = 0;
    
    if (!nc_parser_helper(buffer, tmp_width, tmp_height, tmp_framerate)){ return false; }
    
    // parse mode name
    freenect_video_format tmp_video = FREENECT_VIDEO_DUMMY;
    if (strcmp(buffer, "RGB") == 0){
        tmp_video = FREENECT_VIDEO_RGB;
    } else if (strcmp(buffer, "IR_10BIT") == 0){
        tmp_video = FREENECT_VIDEO_IR_10BIT;
    } else if (strcmp(buffer, "IR_8BIT") == 0){
        tmp_video = FREENECT_VIDEO_IR_8BIT;
    } else {
        return false;
    }

    // find matching    
    int modes_count = freenect_get_video_mode_count();
    for (int i = 0; i < modes_count; ++i){
        freenect_frame_mode current = freenect_get_video_mode(i);
        if (
            (current.video_format == tmp_video) &&
            (current.width == tmp_width) &&
            (current.height == tmp_height) &&
            (current.framerate == tmp_framerate)
        ) {
            format = tmp_video;
            resolution = current.resolution;
            return true;
        }
    }
    
    return false;
}
std::string nc_unparse_video_mode(freenect_video_format format, int width, int height, int fps){
    std::stringstream sx;
    
    switch (format){
        case FREENECT_VIDEO_RGB: { sx << "RGB"; break; }
        case FREENECT_VIDEO_IR_10BIT: { sx << "IR_10BIT"; break; }
        case FREENECT_VIDEO_IR_8BIT: { sx << "IR_8BIT"; break; }
        default: { sx << "UNSUPPORTED"; }
    }
    
    sx << ":" << width << "x" << height << ":" << fps;
    return sx.str();
}
bool nc_parse_depth_mode(const char * mode, freenect_depth_format & format, freenect_resolution & resolution){
    char buffer[40];
    memset(buffer, 0, 40);
    strncpy(buffer, mode, 39);

    int tmp_width = 0;
    int tmp_height = 0;
    int tmp_framerate = 0;
    
    if (!nc_parser_helper(buffer, tmp_width, tmp_height, tmp_framerate)){ return false; }

    // parse mode name
    freenect_depth_format tmp_depth = FREENECT_DEPTH_DUMMY;
    if (strcmp(buffer, "10BIT") == 0){
        tmp_depth = FREENECT_DEPTH_10BIT;
    } else if (strcmp(buffer, "11BIT") == 0){
        tmp_depth = FREENECT_DEPTH_11BIT;
    } else if (strcmp(buffer, "REGISTERED") == 0){
        tmp_depth = FREENECT_DEPTH_REGISTERED;
    } else {
        return false;
    }
    
    // find matching    
    int modes_count = freenect_get_depth_mode_count();
    for (int i = 0; i < modes_count; ++i){
        freenect_frame_mode current = freenect_get_depth_mode(i);
        if (
            (current.depth_format == tmp_depth) &&
            (current.width == tmp_width) &&
            (current.height == tmp_height) &&
            (current.framerate == tmp_framerate)
        ) {
            format = tmp_depth;
            resolution = current.resolution;
            return true;
        }
    }

    return false;
}    
std::string nc_unparse_depth_mode(freenect_depth_format format, int width, int height, int fps){
    std::stringstream sx;
    
    switch (format){
        case FREENECT_DEPTH_10BIT: { sx << "10BIT"; break; }
        case FREENECT_DEPTH_11BIT: { sx << "11BIT"; break; }
        case FREENECT_DEPTH_REGISTERED: { sx << "REGISTERED"; break; }
        default: { sx << "UNSUPPORTED"; }
    }
    
    sx << ":" << width << "x" << height << ":" << fps;
    return sx.str();
}

