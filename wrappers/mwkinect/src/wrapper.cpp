#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <kerat/kerat.hpp>

//#include "nodeconfig.hpp"
#include "wrapper.hpp"

struct contour_entry {
    typedef std::vector<cv::Point> point_vector;
    
    double m_contour_area;
    point_vector m_contour_points;
    point_vector m_convex_hull;
};

struct contour_entry_adaptor: public std::unary_function<contour_entry, const std::vector<cv::Point> &> {
private:
    double m_area_min;
    double m_area_max;

public:
    contour_entry_adaptor(double area_min, double area_max):m_area_min(area_min), m_area_max(area_max){ ; }
    
    contour_entry operator()(const std::vector<cv::Point> & contour) const {
        contour_entry retval;
        retval.m_contour_area = cv::contourArea(contour);;

        if ((retval.m_contour_area >= m_area_min) && (retval.m_contour_area <= m_area_max)){
            cv::approxPolyDP(cv::Mat(contour), retval.m_contour_points, 3, true);
            cv::convexHull(retval.m_contour_points, retval.m_convex_hull);
        }

        return retval;
    }
};


std::vector<cv::Point> ce_extract_contour(const contour_entry & contour){
    return contour.m_contour_points;
}

std::vector<cv::Point> ce_extract_hull(const contour_entry & contour){
    return contour.m_convex_hull;
}

void get_contours(cv::Mat & image, const contour_entry_adaptor & adaptor, std::vector<contour_entry> & output, std::vector<cv::Vec4i> & hierarchy){
/*    // erase all contours that are too small
    const size_t index_next = 0;
    const size_t index_previous = 1;
    const size_t index_child = 2;
    const size_t index_parent = 3;
*/

    std::vector<std::vector<cv::Point> > contours;
    typedef std::vector<cv::Point> point_vector;
    cv::findContours(image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    // aproximated contours
    output.resize(contours.size());
    std::transform(contours.begin(), contours.end(), output.begin(), adaptor);
}

kinect_wrapper::kinect_wrapper(const wrapper_core_config & config)
    :m_config(config),
    m_tuio_server(
        config.target_addr, config.app_name, 
        config.local_ip, config.instance, 
        config.virtual_sensor_width, config.virtual_sensor_height
    ),
    m_dtuio_sa(libkerat::adaptors::append_adaptor::message_list(), libkerat::adaptors::append_adaptor::message_list(), 1)
{
    apply_config(config);

    // setup dtuio
    m_tuio_server.add_adaptor(&m_dtuio_sa);
    
    // send pre-connect echo, serves as recovery message against false contacts
    //m_tuio_server.send();
    commit();
    m_dtuio_sa.set_update_interval(7);
}

kinect_wrapper::~kinect_wrapper(){
    m_tuio_server.clear_session_registry();
    commit();
}

cv::Mat kinect_wrapper::track_hand_candidates(const cv::Mat & input) const {
    cv::Mat retval;
    
//    this belongs to config
    const float hungry_offset = 0;
    const float step_size = 36 * (1.0/65535.0);
    const float fill_color = 1.0;

    cv::Mat tmp = input.clone();
    for (int i = 0; i < tmp.rows; i++){
        for (int j = 0; j < tmp.cols; j++){
            float & pixel = tmp.at<float>(i, j);
            if ((pixel >= m_config.depth_threshold_min + hungry_offset)
                && (pixel <= m_config.depth_threshold_max)
            ) {
                // propagate the valid-detected objects through the corresponding areas
                cv::floodFill(tmp, cv::Point(i, j), fill_color, NULL, 0, step_size);
            }
        }
    }
    tmp.copyTo(retval, tmp == fill_color);

    return retval;
}

libkerat::server * kinect_wrapper::get_tuio_server(){
    return &m_tuio_server;
}

void kinect_wrapper::commit(){
    m_tuio_server.send();
}

bool kinect_wrapper::apply_config(const wrapper_core_config& config){
    //! \todo make this spin-locked
    m_config = config;
    //! \todo safe session ending
    libkerat::message::frame tmp_frame(0);
    tmp_frame.set_address(config.local_ip);
    tmp_frame.set_instance(config.instance);
    tmp_frame.set_app_name(config.app_name);
    tmp_frame.set_sensor_width(config.virtual_sensor_width);
    tmp_frame.set_sensor_height(config.virtual_sensor_height);
    
    m_tuio_server.append_clone(&tmp_frame);
    
    return true;
}

void kinect_wrapper::imprint_tracks(){
#    
    for (cvb::CvTracks::const_iterator track = m_cv_tracks.begin(); track != m_cv_tracks.end(); track++){
        sid_blob_map::const_iterator registered = m_id_mapping.find(track->first);
        libkerat::session_id_t sid = 0;

        if (registered == m_id_mapping.end()){
            sid = m_tuio_server.get_auto_session_id();
            m_id_mapping[track->first] = sid;
        } else {
            sid = registered->second;
        }

/*
        geometry_point pt = transformations.transform(track->second->centroid);

        libkerat::distance_t width = transformations.scale_x(track->second->maxx - track->second->minx);
        libkerat::distance_t height = transformations.scale_y(track->second->maxy - track->second->miny);
        libkerat::angle_t angle = 0;
        // zero radians means that width is the main axis, however if the
        // height is greater than width than angle is pi/2 and width should
        // be exchanged with height
        if (width<height){
            angle = acos(-1)/2;
            double tmp = width;
            width = height;
            height = tmp;
        } 

        width+=40;

        libkerat::message::bounds msg_bnd(sid, 
            pt.x, pt.y,
            angle,
            width, height,
            width*height
        );
        libkerat::message::pointer msg_ptr(sid, 0, 0, 0, pt.x, pt.y, 0, 0); 
        msg_ptr.set_message_output_mode(libkerat::helpers::message_output_mode::OUTPUT_MODE_2D);         

        tuio_server->append_clone(&msg_ptr);
        tuio_server->append_clone(&msg_bnd);
  */
    }
    libkerat::session_set to_remove;
    for (sid_blob_map::const_iterator sid = m_id_mapping.begin(); sid != m_id_mapping.end(); sid++){
        cvb::CvTracks::const_iterator track = m_cv_tracks.find(sid->first);
        if (track == m_cv_tracks.end()){
            to_remove.insert(sid->first);
        }
    }
    for (libkerat::session_set::const_iterator rsid = to_remove.begin(); rsid != to_remove.end(); rsid++){
        m_tuio_server.unregister_session_id(m_id_mapping[*rsid]);
        m_id_mapping.erase(*rsid);
    }
    
}

bool kinect_wrapper::process(cv::Mat rgb, cv::Mat infra, cv::Mat & progress){

    cv::Mat progress_u8c3;
    
    cv::Mat infra_low_thresholded(rgb.size(), CV_32SC1);
    cv::Mat infra_high_thresholded(rgb.size(), CV_32SC1);;
    
    // scale infra to rgb's size
    cv::Mat infra_32bit(rgb.size(), CV_32SC1);
    cv::Mat aux(rgb.size(), CV_32F);
    cv::resize(infra, aux, aux.size());
    aux.convertTo(infra_32bit, CV_32SC1, 65535.0);
    aux.create(rgb.size(), CV_32FC3);

    // apply tresholds
    infra_low_thresholded = infra_32bit;
    infra_low_thresholded.setTo(0, infra_low_thresholded < m_config.depth_threshold_min);
    infra_low_thresholded.copyTo(infra_high_thresholded);
    infra_high_thresholded.setTo(0, infra_high_thresholded > m_config.depth_threshold_max);

    // create progress_u8c3
    rgb.convertTo(progress_u8c3, CV_8UC3, 255.0/1.0, 0);
    
    bool retval = true;
    //retval = process_internal(infra_low_thresholded, infra_high_thresholded, progress_u8c3);

    // output progress
    progress_u8c3.convertTo(progress, CV_32FC3, 1.0/255.0, 0);
    rgb.copyTo(progress);

    return retval;
}

bool kinect_wrapper::process_internal(cv::Mat infra_low __attribute__((unused)), cv::Mat infra_high, cv::Mat & progress_8uc3){
    
    cv::Mat infra_high_mask;
    infra_high.convertTo(infra_high_mask, CV_8UC1, 0, 255);
    
#if 0
    { // debug blok
        cv::Mat tmp[] = {infra_high_thresholded, infra_high_thresholded, infra_high_thresholded};
        cv::Mat output;
        cv::merge(tmp, 3, output);
        output.convertTo(progress, CV_32FC3, 1.0/255.0);
    }
#endif

    //std::fstream of("/tmp/kinect_data", std::ios_base::out | std::ios_base::binary);
    //of.write((char *)infra_high_thresholded.datastart, infra_high_thresholded.dataend - infra_high_thresholded.datastart);
    //of.close();

    
    //! \todo expand the valid contact-blobs onto larger blobs used for hand-body tracking
    //cv::Mat infra_hands = track_hand_candidates(infra_low_thresholded);
    //infra_hands.depth();

    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(infra_high_mask, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    
    std::vector<std::vector<cv::Point> >::iterator i = contours.begin();
    while (i != contours.end()){
        double area = cv::contourArea(*i);
        if ((m_config.blob_area_min <= area) && (area <= m_config.blob_area_max)){
            ++i;
            continue;
        }
        contours.erase(i);
    }

#if 0    
    typedef std::vector<contour_entry> contour_vector;
    contour_vector contours_updated;
    
    //get_contours(infra_high_thresholded, adaptor, contours_updated, hierarchy);
    
    typedef std::vector<cv::Point> point_vector;
    
    for (std::vector<std::vector<cv::Point> >::iterator i = contours.begin(); i != contours.end(); ++i){
        for (std::vector<cv::Point>::iterator j = i->begin(); j != i->end(); ++j){
            std::cout << j->x << "," << j->y << " ";
        }
        std::cout << std::endl;
        //std::vector<cv::Point> tmp = *i;
        //cv::approxPolyDP(cv::Mat(tmp), *i, 3, true);
        //cv::convexHull(tmp, *i, true, true);
    }
#endif    

    
    cv::drawContours(progress_8uc3, contours, -1, cv::Scalar(255, 0, 0), 3, 8);

    return true;
}
