//
// Created by wouter on 5/15/18.
//


#include "PluginAlternateColorDetection.h"


struct p_dist {
    uchar white;
    uchar black;
    uchar yellow;
    uchar blue;
    uchar green;
    uchar purple;
};

PluginAlternateColorDetection::PluginAlternateColorDetection(FrameBuffer *_buffer) : VisionPlugin(_buffer) {

}

ProcessResult PluginAlternateColorDetection::process(FrameData *data, RenderOptions *options) {
    (void)options;

//    cv::Mat* img_hsv;
//    if ((img_hsv=(cv::Mat *)data->map.get("cmv_threshold")) == 0) {
//        img_hsv = new cv::Mat(data->video.getHeight(),
//                              data->video.getWidth(),
//                              CV_8UC3,
//                              new uchar[data->video.getHeight()*data->video.getWidth()*3]);
//        data->map.insert("cmv_threshold",img_hsv);
//    }

    if (data->video.getColorFormat()==COLOR_YUV422_UYVY) {
        fprintf(stderr,"This shit needs RGB8 as input image, but found: %s\n",Colors::colorFormatToString(data->video.getColorFormat()).c_str());
        return ProcessingFailed;
    } else if (data->video.getColorFormat()==COLOR_YUV444) {
        fprintf(stderr,"This shit needs RGB8 as input image, but found: %s\n",Colors::colorFormatToString(data->video.getColorFormat()).c_str());
        return ProcessingFailed;
    } else if (data->video.getColorFormat()==COLOR_RGB8) {
        cv::Mat img(
                data->video.getHeight(),
                data->video.getWidth(),
                CV_8UC3,
                data->video.getData());
        cv::Mat img_hsv = img.clone();
        fprintf(stdout, "converting...\n");
        fflush(stdout);
        cv::cvtColor(img, img_hsv, CV_RGB2HSV);
        fprintf(stdout, "processing...\n");
        fflush(stdout);
        processHSV(img_hsv);
        return ProcessingOk;
    } else {
        fprintf(stderr,"This shit needs RGB8 as input image, but found: %s\n",Colors::colorFormatToString(data->video.getColorFormat()).c_str());
        return ProcessingFailed;
    }
}

void PluginAlternateColorDetection::processHSV(cv::Mat img_hsv) {
    uchar* hsvdata = (img_hsv).data;
    int rows = (img_hsv).rows, cols = (img_hsv).cols, size = rows*cols;
    p_dist converted[size];
    for (int i = 0; i < size*3; i+=3) {
        uchar h = hsvdata[i];
        uchar s = hsvdata[i + 1];
        uchar v = hsvdata[i + 2];
        int j = i/3;
        converted[j].black = v
    }
}
