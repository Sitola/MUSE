#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <opencv2/opencv.hpp>
#include "cv_common.hpp"

const size_t HEATMAP_KINECT_INTOLERANT = 60000;

void depth_to_heatmap(cv::Mat& in, cv::Mat& out, double & global_minimum, double & global_maximum){
    cv::Mat tmp;

#ifdef MWKINECT_ADAPTIVE_HEATMAP
    { // scale the heatmap
        double minimum = global_minimum;
        double maximum = global_maximum;

        in.copyTo(tmp, in < HEATMAP_KINECT_INTOLERANT);
        cv::minMaxIdx(tmp, &minimum, &maximum);
        
        global_minimum = std::min(minimum, global_minimum);
        global_maximum = std::max(maximum, global_maximum);
        
        if (global_minimum == global_maximum) { ++global_maximum; }
    }
#endif
    
    in.convertTo(tmp, CV_32F, 360.0/(global_maximum - global_minimum), -(360.0*global_minimum)/(global_maximum - global_minimum));

    out.release();
    out.create(in.size(), CV_32FC3);
    cv::Mat zr = cv::Mat::ones(in.size(), CV_32F);
    cv::Mat inputs[] = { zr, tmp };
    int mappings[] = { 0, 1, 0, 2, 1, 0};
    
    cv::Mat tmp2(in.size(), CV_32FC3);
    cv::mixChannels(inputs, 2, &tmp2, 1, mappings, 3);
    
    cv::cvtColor(tmp2, out, CV_HSV2RGB);
}