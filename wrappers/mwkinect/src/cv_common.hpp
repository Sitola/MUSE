/**
 * \file      cv_common.hpp
 * \brief     Headers for common opencv related shortcuts
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-04 12:57 UTC+2
 * \copyright BSD
 */

#ifndef MWKINECT_CV_COMMON_HPP
#define	MWKINECT_CV_COMMON_HPP

#include <opencv2/opencv.hpp>

void depth_to_heatmap(cv::Mat & in, cv::Mat & out, double & global_minimum, double & global_maximum);

#endif	/* MWKINECT_CV_COMMON_HPP */

