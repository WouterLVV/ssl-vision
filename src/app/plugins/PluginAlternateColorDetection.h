//
// Created by wouter on 5/15/18.
//

#ifndef SSL_VISION_PLUGINALTERNATECOLORDETECTION_H
#define SSL_VISION_PLUGINALTERNATECOLORDETECTION_H

#include "visionplugin.h"
#include <framedata.h>
#include <lcms.h>
#include <VarNotifier.h>
#include "opencv2/opencv.hpp"
#include "cmvision_region.h"

class p_dist {
public:
    bool used = false;
    //uchar white;
    //uchar black;
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


static const string str_time_thresh = "TIME_THRESHOLD";
static const string str_time_class  = "TIME_CLASSIFY";

class PluginAlternateColorDetection : public VisionPlugin {

public:
    explicit PluginAlternateColorDetection(FrameBuffer *_buffer, YUVLUT* lut);

    ProcessResult process(FrameData *data, RenderOptions *options) override;

    string getName() override;

    VarList* getSettings() override;

    p_dist *processHSV(cv::Mat img_hsv, uchar *hlut, p_dist *converted);

    uchar probgaussianH(int a, int b, int c, int x);

    uchar probgaussianV(int a, int b, int c, int x);

    void buildHlut(uchar *lut);

    blob makeblob(p_dist *converted, int pixel, int cols, int size);

    void blobdetection(p_dist *converted, int cols, int size, vector<blob> *blobs);



protected:
    bool enabled = true;

    YUVLUT* _lut;
    VarList* _settings;
    VarNotifier _notifier;

    int greencenter = 60;
    int greendev = 20;
    int pinkcenter = 15;
    int pinkdev = 20;
    int yellowcenter = 30;
    int yellowdev = 20;
    int bluecenter = 120;
    int bluedev = 20;
    int blackcenter = 0;
    int blackdev = 50;
    //int a = 255;

    uchar maxHdist = 30;
    uchar maxVdist = 90;

    int hor_regions = 3;
    int vert_regions = 3;
    int num_regions = hor_regions * vert_regions;
    int avgradius = 4;

    int minpixels = 20;
    int maxpixels = 30;

    uchar* hlut;



};


#endif //SSL_VISION_PLUGINALTERNATECOLORDETECTION_H
