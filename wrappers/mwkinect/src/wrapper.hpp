/**
 * \file wrapper.hpp
 * \brief     The main library header file. Loads the individual headers.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-04 12:57 UTC+2
 * \copyright BSD
 */

#ifndef MWKINECT_WRAPPER_HPP
#define	MWKINECT_WRAPPER_HPP

#include <opencv2/opencv.hpp>
#include <cvblob.h>
#include <kerat/kerat.hpp>
#include <map>

#include "nodeconfig.hpp"

class kinect_wrapper {
public:
    kinect_wrapper(const wrapper_core_config & config);
    ~kinect_wrapper();
    
    bool apply_config(const wrapper_core_config & config);
    
    libkerat::server * get_tuio_server();
    
    void commit();
    
    bool process(cv::Mat rgb, cv::Mat infra, cv::Mat & progress);
    
private:
    typedef std::map<size_t, libkerat::session_id_t> sid_blob_map;    
    
    void imprint_tracks();
    
    cv::Mat track_hand_candidates(const cv::Mat & input) const;
    bool process_internal(cv::Mat infra_low, cv::Mat infra_high, cv::Mat & progress_8uc3);

    wrapper_core_config m_config;
    static const size_t m_maximum_contours_count = 100;
    
    libkerat::simple_server m_tuio_server;
    libkerat::adaptors::append_adaptor m_dtuio_sa;

    sid_blob_map m_id_mapping;
    
    cvb::CvTracks m_cv_tracks;
    
};

#endif	/* MWKINECT_WRAPPER_HPP */

