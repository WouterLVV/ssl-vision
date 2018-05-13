//
// Created by wouter on 4/26/18.
//

#include <messages_robocup_ssl_detection.pb.h>
#include <image.h>
#include "plugin_stats.h"

PluginStats::PluginStats(FrameBuffer *_buffer) : VisionPlugin(_buffer) {

}

ProcessResult PluginStats::process(FrameData *data, RenderOptions *options) {
    SSL_DetectionFrame * detection_frame = 0;

    detection_frame=(SSL_DetectionFrame *)data->map.get("ssl_detection_frame");
    if (detection_frame == 0) return ProcessingFailed;

    Image<raw8> * img_thresholded;

    if ((img_thresholded=(Image<raw8> *)data->map.get("cmv_threshold")) == 0) {
        return ProcessingFailed;
    }

    printf("blues:\n");
    auto blues = detection_frame->robots_blue();
    for (int i = 0; i < blues.size(); i++) {
        SSL_DetectionRobot bot = blues[i];
        for (int j = 0; j < bot.clrs_size(); j++) {
            raw8 color;
            color.set(bot.clrs(j));
            unsigned int x1 = bot.x1s(j),x2 = bot.x2s(j),y1 = bot.y1s(j),y2 = bot.y2s(j);
            printf("blob %u:\ncolor: %u, x1: %u, x2: %u, y1: %u, y2: %u\n", j, color.v, x1, x2, y1, y2);

        }

    }
    printf("yellows:\n");
    auto yellows = detection_frame->robots_yellow();
    for (int i = 0; i < yellows.size(); i++) {
    SSL_DetectionRobot bot = yellows[i];
        for (int j = 0; j < bot.clrs_size(); j++) {
            raw8 color;
            color.set(bot.clrs(j));
            unsigned int x1 = bot.x1s(j),x2 = bot.x2s(j),y1 = bot.y1s(j),y2 = bot.y2s(j);
            printf("blob %u:\ncolor: %u, x1: %u, x2: %u, y1: %u, y2: %u\n", j, color.v, x1, x2, y1, y2);

        }

    }

    return ProcessingOk;
}
