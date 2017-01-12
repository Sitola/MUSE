/**
 * \file      kinect_device.cpp
 * \brief     Provide the kinect device and the device-related core operations
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \author    Vít Rusňák <xrusnak@fi.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-08-07 20:10 UTC+2
 * \copyright BSD
 */

#include <errno.h>
#include <pthread.h>
#include <iostream>
#include <libfreenect/libfreenect.h>

#include "kinect_device.hpp"
#include "gtk_ui.hpp"
//#include <libfreenect/libfreenect_cv.h>

//cv::Mat mat = cv::Mat(image->height, image->width, CV_8UC3, image->data);cv::Mat mat = cv::Mat(image->height, image->width, CV_8UC3, image->data);

kinect_device::kinect_device(const kinect_device_config & config)
    :m_kinect_context(config.kinect_context), m_kinect_device(NULL),
//    m_gamma(2048), 
    m_new_rgb_frame(false), 
    m_new_depth_frame(false),
    m_valid_config(false),
    m_undissort(false)
{
    assert(m_kinect_context != NULL);
    
    int err = freenect_open_device_by_camera_serial(m_kinect_context, &m_kinect_device, config.device_serial.c_str());
    if (err != 0){ throw std::invalid_argument("Unable to open given kinect device!"); }
    
    freenect_set_user(m_kinect_device, this);
    freenect_set_led(m_kinect_device, LED_BLINK_RED_YELLOW);
    freenect_set_video_callback(m_kinect_device, video_callback);
    freenect_set_depth_callback(m_kinect_device, depth_callback);
    
}

kinect_device::~kinect_device() {
    if (m_kinect_device != NULL) {
        freenect_set_led(m_kinect_device, LED_RED);
        freenect_close_device(m_kinect_device);
        m_kinect_device = NULL;
    }
}

bool kinect_device::apply_config(const kinect_device_config & config){
    
    cv::Mat tmp_rgb_matrix;
    cv::Mat tmp_depth_matrix;
    
    // check video mode
    freenect_frame_mode video_mode = freenect_find_video_mode(config.kinect_camera_resolution, config.kinect_camera_format);

    size_t kinect_rgb_pixel_bytes = (video_mode.data_bits_per_pixel + video_mode.padding_bits_per_pixel + 7)/8;
    if (kinect_rgb_pixel_bytes != 3){
        std::cerr << "RGB video has to have exactly 3 bytes per pixel!" << std::endl;
        return false; 
    }
    
    if (video_mode.is_valid == 0){ 
        std::cerr << "Invalid kinect video format " << config.kinect_camera_format << " with resolution " << config.kinect_camera_resolution << std::endl;
        return false; 
    }
    
    // check depth mode
    freenect_frame_mode depth_mode = freenect_find_depth_mode(config.kinect_depth_resolution, config.kinect_depth_format);
    if (depth_mode.is_valid == 0){ 
        std::cerr << "Invalid kinect depth format " << config.kinect_depth_format << " with resolution " << config.kinect_depth_resolution << std::endl;
        return false; 
    }
    
    // create video matrix buffer
    tmp_rgb_matrix.create(
        video_mode.width, video_mode.height, 
        CV_8UC3
    );
    if ((tmp_rgb_matrix.dataend - tmp_rgb_matrix.datastart) < video_mode.bytes){
        std::cerr << "Unable to set corresponding opencv video frame format!" << std::endl;
        return false; 
    }

    // create depth matrix buffer
    tmp_depth_matrix.create(
        depth_mode.width, depth_mode.height, 
        CV_16U
    );
    if ((tmp_depth_matrix.dataend - tmp_depth_matrix.datastart) < depth_mode.bytes){
        std::cerr << "Unable to set corresponding opencv depth frame format!" << std::endl;
        return false; 
    }
    
    //! \todo zjisti proc undistort selhava
    cv::Mat tmp_target(tmp_rgb_matrix.size.operator ()(), CV_32FC3);
    try {
        cv::undistort(tmp_target, tmp_rgb_matrix, config.camera_matrix_rgb, config.distortion_matrix_rgb);    
    } catch (const cv::Exception & ex) {
        std::cerr << ex.what() << std::endl;
        return false;
    }
    
    tmp_target.create(tmp_depth_matrix.size.operator ()(), CV_32F);
    try {
        cv::undistort(tmp_target, tmp_depth_matrix, config.camera_matrix_ir, config.distortion_matrix_ir);    
    } catch (const cv::Exception & ex) {
        std::cerr << ex.what() << std::endl;
        return false;
    }

    // commance setup
    freenect_stop_depth(m_kinect_device);
    freenect_stop_video(m_kinect_device);
    freenect_set_depth_mode(m_kinect_device, depth_mode);
    freenect_set_video_mode(m_kinect_device, video_mode);

    // load distortion matrices
    m_camera_matrix_rgb = config.camera_matrix_rgb;
    m_camera_matrix_ir = config.camera_matrix_ir;
    m_distortion_matrix_rgb = config.distortion_matrix_rgb;
    m_distortion_matrix_ir = config.distortion_matrix_ir;
    // setup buffer matrices
    m_rgb_matrix.create(
        video_mode.width, video_mode.height, 
        CV_8UC3
    );
    m_depth_matrix.create(
        depth_mode.width, depth_mode.height, 
        CV_16U
    );
    
    freenect_set_video_buffer(m_kinect_device, m_rgb_matrix.data);
    freenect_set_depth_buffer(m_kinect_device, m_depth_matrix.data);
    
    std::cerr << "Running with video=" << video_mode.width << "x" << video_mode.height
        << " type=" << video_mode.video_format << " and depth="
        << depth_mode.width << "x" << depth_mode.height << " type=" 
        << video_mode.depth_format << std::endl;
    
    m_undissort = config.transform_input_image;
    
    // signal ready
    m_valid_config = true;
    freenect_set_led(m_kinect_device, LED_YELLOW);
    
    return true;
}

kinect_device::frame_mode_vector kinect_device::get_video_modes() {
    frame_mode_vector retval;
    
    const int modes_count = freenect_get_video_mode_count();
    for (int i = 0; i < modes_count; ++i){
        freenect_frame_mode mode = freenect_get_video_mode(i);
        // this test is not currently needed, but it is possible that the
        // api for next device version will be about the same...
        freenect_frame_mode verified = freenect_find_video_mode(mode.resolution, mode.video_format);
        if (verified.is_valid) {
            retval.push_back(verified);
        }
    }
    
    return retval;
}
kinect_device::frame_mode_vector kinect_device::get_depth_modes() {
    frame_mode_vector retval;
    
    const int modes_count = freenect_get_depth_mode_count();
    for (int i = 0; i < modes_count; ++i){
        freenect_frame_mode mode = freenect_get_depth_mode(i);
        // this test is not currently needed, but it is possible that the
        // api for next device version will be about the same...
        freenect_frame_mode verified = freenect_find_depth_mode(mode.resolution, mode.depth_format);
        if (verified.is_valid) {
            retval.push_back(verified);
        }
    }
    
    return retval;
}

bool kinect_device::set_depth_mode(freenect_depth_format format, freenect_resolution resolution){
    freenect_frame_mode depth_mode = freenect_find_depth_mode(resolution, format);
    if (!depth_mode.is_valid) { return false; }
    
    freenect_set_depth_mode(m_kinect_device, depth_mode);
    
    freenect_frame_mode tested_mode = freenect_get_current_depth_mode(m_kinect_device);
    return (tested_mode.resolution == resolution) && (tested_mode.depth_format == format);
}

bool kinect_device::set_video_mode(freenect_video_format format, freenect_resolution resolution){
    freenect_frame_mode video_mode = freenect_find_video_mode(resolution, format);
    if (!video_mode.is_valid) { return false; }
    
    freenect_set_video_mode(m_kinect_device, video_mode);
    
    freenect_frame_mode tested_mode = freenect_get_current_video_mode(m_kinect_device);
    return (tested_mode.resolution == resolution) && (tested_mode.video_format == format);
}

// Do not call directly even in child
void kinect_device::video_callback(freenect_device *dev, void *rgb __attribute__((unused)), uint32_t timestamp __attribute__((unused))) {
    kinect_device * that = (kinect_device *)(freenect_get_user(dev));
    that->m_new_rgb_frame = true;
}

// Do not call directly even in child
void kinect_device::depth_callback(freenect_device *dev, void *depth __attribute__((unused)), uint32_t timestamp __attribute__((unused))) {
    kinect_device * that = (kinect_device *)(freenect_get_user(dev));
    
#if 0    
    uint8_t* rgb = static_cast<uint8_t*>(rgb);
    memcpy(m_rgb_matrix.data, rgb, m_rgb_matrix.cols*m_rgb_matrix.rows*m_rgb_matrix.depth());

    // compensate for distorsions
    cv::Mat temp = m_rgb_matrix.clone();
    cv::undistort(temp, m_rgb_matrix, m_camera_matrix_rgb, m_distortion_matrix_rgb);    
#endif
    
    that->m_new_depth_frame = true;
}

bool kinect_device::get_raw_data(cv::Mat& rgb, cv::Mat& depth) {
    if ((m_new_depth_frame && m_new_rgb_frame) == false){ return false; }

    assert(m_valid_config);

    rgb.create(m_rgb_matrix.size.operator ()(), CV_32FC3);
    depth.create(m_depth_matrix.size.operator ()(), CV_32F);
    
    m_rgb_matrix.convertTo(rgb, CV_32FC3, 1.0/255.0);
    m_depth_matrix.convertTo(depth, CV_32F, 1.0/65353.0); // depth is 16b single channel
    
    // emmit frame
    m_new_depth_frame = false;
    m_new_rgb_frame = false;
    
    return true;
}

bool kinect_device::get_camera_data(cv::Mat& rgb, cv::Mat& depth) {
    bool can_proceed = get_raw_data(rgb, depth);
    if (!can_proceed) { return false; }
    
    if (m_undissort){
        cv::Mat tmp(m_rgb_matrix.size.operator ()(), CV_32FC3);
        m_rgb_matrix.convertTo(tmp, CV_32FC3);
        cv::undistort(rgb, tmp, m_camera_matrix_rgb, m_distortion_matrix_rgb);
    
        tmp.create(m_depth_matrix.size.operator ()(), CV_32FC1);
        m_depth_matrix.convertTo(tmp, CV_32FC1);
        cv::undistort(depth, tmp, m_camera_matrix_ir, m_distortion_matrix_ir);
    }
        
    // emmit frame
    return true;
}

bool kinect_device::start(){
    if (m_valid_config) {
        freenect_start_video(m_kinect_device);
        freenect_start_depth(m_kinect_device);
        return true;
    }
    return false;
}

bool kinect_device::stop(){
    if (m_valid_config) {
        freenect_stop_depth(m_kinect_device);
        freenect_stop_video(m_kinect_device);
        return true;
    }
    return false;
}

void kinect_device::process(){
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000;

    freenect_process_events_timeout(m_kinect_context, &tv);
}

void kinect_device::set_led(freenect_led_options led){
    freenect_set_led(m_kinect_device, led);
}