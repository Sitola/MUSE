/**
 * \file      nodeconfig.hpp
 * \brief     Provides the struct that holds the wrapper configuration
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-03-17 19:14 UTC+2
 * \copyright BSD
 */

#ifndef MWKINECT_NODECONFIG_HPP
#define MWKINECT_NODECONFIG_HPP

#include <string>
#include <list>
#include <inttypes.h>
#include <uuid/uuid.h>
#include <opencv2/opencv.hpp>
#include <kerat/kerat.hpp>
#include <libfreenect/libfreenect.h>

#include "kinect_device.hpp"

//#include "geometry_primitives.pp"

#define VERBOSITY_LEVEL_DUMP (1 << 0)
#define DIMMENSION_UNSET ((uint16_t)-1)

// forwards
class kinect_device;

struct kinect_device_config {
    kinect_device_config();
    
    void set_defaults();

    void reset_transformations();
    
    kinect_device_config & operator=(const kinect_device_config & config);
    
    freenect_resolution kinect_camera_resolution;
    freenect_resolution kinect_depth_resolution;
    freenect_video_format kinect_camera_format;
    freenect_depth_format kinect_depth_format;
    bool transform_input_image;
    
	cv::Mat stereo_matrix_r;
    cv::Mat stereo_matrix_t;
	cv::Mat camera_matrix_rgb;
	cv::Mat distortion_matrix_rgb;
	cv::Mat camera_matrix_ir;
	cv::Mat distortion_matrix_ir;
    
    freenect_context * kinect_context;
    std::string device_serial;
    
};

struct sensor_config {
    sensor_config();
    
    void set_defaults();

    sensor_config & operator=(const sensor_config & config);
    
    uuid_t uuid;
    
    int16_t virtual_sensor_width;
    int16_t virtual_sensor_height;

    int depth_threshold_min;
    int depth_threshold_max;
    
    int blob_area_min;
    int blob_area_max;
    
};

struct wrapper_core_config: public sensor_config {
    
    wrapper_core_config();
    
    typedef std::list<libkerat::kerat_message *> dtuio_list;
    dtuio_list prepared_dtuio;

    std::string target_addr;
    std::string app_name;
    uint32_t local_ip;
    uint32_t instance;
};

struct gtk_gui_config {
    std::string m_window_title;
};

class mwkinect_gui;

struct updatable_config: public kinect_device_config, public gtk_gui_config, public wrapper_core_config{
    updatable_config();
    virtual ~updatable_config();

    bool force_update;

    std::string config_path;
};

struct thread_com {
    
    thread_com(updatable_config * cfg);
    
    int argc;
    char ** argv;
    pthread_t gui_thread;
    updatable_config * config;
    pthread_spinlock_t update_lock;

    mwkinect_gui * gui_instance;
    volatile bool keep_running;
};

struct node_config: public updatable_config {
protected:
    node_config();
    ~node_config();
        
    static node_config * m_instance;
public:
    typedef enum {UI_AUTODETECT = -1, UI_UNSET = 0, UI_GTK = 1, UI_CONSOLE = 2} ui_t;
    typedef enum {MODE_WRAPPER, MODE_CALIBRATION, MODE_HELP, MODE_LIST} op_mode_t;
    
    static node_config * get_instance();
    static void destroy();
    
    virtual void set_defaults();

    op_mode_t operation_mode;
    kinect_device * device;
    
    std::string config_path;
    unsigned int verbosity;
    std::string kinect_calib_string;

    bool disable_pidfile;
    std::string pidfile_name;

    ui_t ui;
    thread_com thread_com_channel;
};

bool load_config_file(node_config & config);
bool complete_config(node_config & config);
bool save_config_file(std::ostream & output, const node_config & config);

bool nc_parse_video_mode(const char * mode, freenect_video_format & format, freenect_resolution & resolution);
std::string nc_unparse_video_mode(freenect_video_format format, int width, int height, int fps);
bool nc_parse_depth_mode(const char * mode, freenect_depth_format & format, freenect_resolution & resolution);
std::string nc_unparse_depth_mode(freenect_depth_format format, int width, int height, int fps);
    
#endif // MWKINECT_NODECONFIG_HPP

