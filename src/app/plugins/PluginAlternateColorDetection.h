//
// Created by wouter on 5/15/18.
//

#ifndef SSL_VISION_PLUGINALTERNATECOLORDETECTION_H
#define SSL_VISION_PLUGINALTERNATECOLORDETECTION_H

#include "visionplugin.h"
#include <framedata.h>
#include "opencv2/opencv.hpp"

class PluginAlternateColorDetection : public VisionPlugin {

public:
    explicit PluginAlternateColorDetection(FrameBuffer *_buffer);

    ProcessResult process(FrameData *data, RenderOptions *options) override;

    void processHSV(cv::Mat img_hsv);

    void processHSV(cv::Mat *img_hsv);

    void processHSV(cv::Mat img_hsv, cv::Mat img_rgb);
};


#endif //SSL_VISION_PLUGINALTERNATECOLORDETECTION_H
