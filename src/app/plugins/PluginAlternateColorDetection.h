//
// Created by wouter on 5/15/18.
//

#ifndef SSL_VISION_PLUGINALTERNATECOLORDETECTION_H
#define SSL_VISION_PLUGINALTERNATECOLORDETECTION_H

#include "visionplugin.h"
#include <framedata.h>
#include <lcms.h>
#include "opencv2/opencv.hpp"
#include "cmvision_region.h"

class p_dist {
public:
    bool used = false;
    uchar white;
    uchar black;
    uchar* colors;
    uchar h;
    uchar s;
    uchar v;
    uchar vavg;
};

class blob {
public:
    inline blob(){

    };
    uchar color = 0;
    vector<int> pixels;
    int max_x = 0;
    int min_x = 0;
    int max_y = 0;
    int min_y = 0;
    int size  = 0;

    double cen_x = 0;
    double cen_y = 0;
};


class PluginAlternateColorDetection : public VisionPlugin {

public:
    YUVLUT* _lut;

    explicit PluginAlternateColorDetection(FrameBuffer *_buffer, YUVLUT* lut);

    ProcessResult process(FrameData *data, RenderOptions *options) override;

    p_dist * processHSV(cv::Mat img_hsv, uchar *hlut);

    p_dist *processHSV(cv::Mat img_hsv, uchar *hlut, p_dist *converted);
};


#endif //SSL_VISION_PLUGINALTERNATECOLORDETECTION_H
