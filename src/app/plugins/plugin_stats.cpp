//
// Created by wouter on 4/26/18.
//

#include <messages_robocup_ssl_detection.pb.h>
#include <image.h>
#include <cmvision_region.h>
#include "plugin_stats.h"
#include "PluginAlternateColorDetection.h"
#include <ctime>

PluginStats::PluginStats(FrameBuffer *_buffer) : VisionPlugin(_buffer) {
    _settings = new VarList("Statistics");
    _settings->addChild(new VarBool("Enabled", false));
    _settings->addChild(new VarInt("Frames to record",1 ,1));
    _settings->addChild(new VarString("Title", "bots"));

    _notifier.addRecursive(_settings);

}

VarList* PluginStats::getSettings() {
    return _settings;
}

ProcessResult PluginStats::process(FrameData *data, RenderOptions *options) {
    (void)options;

    if (_notifier.hasChanged()) {
        stats.reset();
        enabled = ((VarBool*)_settings->findChild("Enabled"))->getBool();
        target_frames = ((VarInt*)_settings->findChild("Frames to record"))->getInt();
        title = ((VarString*)_settings->findChild("Title"))->getString();
        written = false;
    }

    if (!enabled || written) return ProcessingOk;

    if (stats.framecount >= target_frames) {


        std::time_t now = std::time(NULL);
        std::tm * ptm = std::localtime(&now);
        char filename[100];
// Format: Mo, 15.06.2009 20:20:00
        std::strftime(filename,100, "timings/TIMINGS_%Y%m%d%H%M%S.txt", ptm);
        ofstream output(filename);

        char str[10000];
        sprintf(str,
                "--------------\n"
                "Total frames: %d\n"
                "Amount of robots seen: %lu\n",
                stats.framecount,
                stats.botcountsblue.size() + stats.botcountsyellow.size()
        );
        fprintf(stdout, str);
        output << title << endl << str;
        for (pair<int, int> p : stats.botcountsyellow) {
            sprintf(str,
                    "Amount of Y%d: %d\n",
                    p.first, p.second
            );
            fprintf(stdout, str);
            output << str;
        }
        for (pair<int, int> p : stats.botcountsblue) {
            sprintf(str,
                    "Amount of B%d: %d\n",
                    p.first, p.second
            );
            fprintf(stdout, str);
            output << str;
        }


        sprintf(str,
                "Average timing Alt total: %f\n"
                "Average timing Alt thresholding: %f\n"
                "Average timing Alt classifying: %f\n"
                "Average timing Std total: %f\n"
                "Average timing Std thresholding: %f\n"
                "Average timing Std runlength: %f\n"
                "Average timing Std find blobs: %f\n"
                "-----------------------\n",
                stats.cumulative_timings["Alternative Detection"] * 1000.0 /(double)stats.framecount,
                stats.cumulative_timings[str_time_thresh] * 1000.0 /(double)stats.framecount,
                stats.cumulative_timings[str_time_class] * 1000.0 /(double)stats.framecount,
                (stats.cumulative_timings["Segmentation"] + stats.cumulative_timings["RunlengthEncode"] + stats.cumulative_timings["FindBlobs"]) * 1000.0 /(double)stats.framecount,
                stats.cumulative_timings["Segmentation"] * 1000.0 /(double)stats.framecount,
                stats.cumulative_timings["RunlengthEncode"] * 1000.0 /(double)stats.framecount,
                stats.cumulative_timings["FindBlobs"] * 1000.0 /(double)stats.framecount
        );
        fprintf(stdout, str);
        output << str;
        output.close();
        fflush(stdout);
        written = true;
        return ProcessingOk;
    }

    unordered_map<string, double>* timings;
    if ((timings = (unordered_map<string, double>*)data->map.get("timings")) == 0) {
        return ProcessingFailed;
    }

    SSL_DetectionFrame * detection_frame = 0;

    detection_frame=(SSL_DetectionFrame *)data->map.get("ssl_detection_frame");
    if (detection_frame == 0) return ProcessingFailed;

    Image<raw8> * img_thresholded;

    if ((img_thresholded=(Image<raw8> *)data->map.get("cmv_threshold")) == 0) {
        return ProcessingFailed;
    }

    CMVision::ColorRegionList * colorlist;
    if ((colorlist=(CMVision::ColorRegionList *)data->map.get("cmv_colorlist")) == 0) {
        return ProcessingFailed;
    }

    stats.framecount++;
    FrameStats frame;





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
        frame.bluebots++;
        frame.robottotal++;
        BotStats bs;
        bs.id = bot.robot_id();
        bs.team = 0;
        bs.x = bot.x();
        bs.y = bot.y();
        frame.bots[bs.id] = bs;
        stats.botcountsblue[bs.id]+=1;

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
        frame.yellowbots++;
        frame.robottotal++;
        BotStats bs;
        bs.id = bot.robot_id();
        bs.team = 1;
        bs.x = bot.x();
        bs.y = bot.y();
        frame.bots[bs.id] = bs;
        stats.botcountsyellow[bs.id]+=1;

    }

    for (pair<string, double> p : (*timings)) {
        frame.timings[p.first] = p.second;
        stats.cumulative_timings[p.first] += p.second;
    }

    stats.frames.push_back(frame);

    return ProcessingOk;
}
