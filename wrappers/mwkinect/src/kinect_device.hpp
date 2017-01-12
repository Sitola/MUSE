/**
 * \file      kinect_device.hpp
 * \brief     Provide the class that represents the kinect device and the device-related core operations
 * \author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \author    Vít Rusňák <xrusnak@fi.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-08-07 20:10 UTC+2
 * \copyright BSD
 */

#ifndef MWKINECT_KINECT_DEVICE_HPP
#define MWKINECT_KINECT_DEVICE_HPP

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <pthread.h>

#include <libfreenect/libfreenect.h>

#include "nodeconfig.hpp"

class kinect_device_config;

class kinect_device {

public:
    typedef std::vector<freenect_frame_mode> frame_mode_vector;

    kinect_device(const kinect_device_config & config);
    ~kinect_device();
    
    bool apply_config(const kinect_device_config & config);
    
    frame_mode_vector get_video_modes();
    frame_mode_vector get_depth_modes();
    
    bool set_depth_mode(freenect_depth_format format, freenect_resolution resolution);
    bool set_video_mode(freenect_video_format format, freenect_resolution resolution);

    bool get_raw_data(cv::Mat& rgb, cv::Mat & depth);
    bool get_camera_data(cv::Mat& rgb, cv::Mat & depth);
    
    bool start();
    bool stop();
    void process();
    
    void set_led(freenect_led_options led);
    
private:

    static void video_callback(freenect_device *dev, void *rgb, uint32_t timestamp);
    static void depth_callback(freenect_device *dev, void *depth, uint32_t timestamp);
    
    freenect_context * m_kinect_context;
    freenect_device * m_kinect_device;

	cv::Mat m_stereo_matrix_r;
    cv::Mat m_stereo_matrix_t;
	cv::Mat m_camera_matrix_rgb;
	cv::Mat m_distortion_matrix_rgb;
	cv::Mat m_camera_matrix_ir;
	cv::Mat m_distortion_matrix_ir;
    
    bool m_new_rgb_frame;
    bool m_new_depth_frame;
    bool m_valid_config;
    bool m_undissort;
    
    cv::Mat m_depth_matrix;
    cv::Mat m_rgb_matrix;
};

#endif // MWKINECT_KINECT_DEVICE_HPP
