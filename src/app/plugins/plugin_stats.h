//
// Created by wouter on 4/26/18.
//

#ifndef SSL_VISION_PLUGINSTATS_H
#define SSL_VISION_PLUGINSTATS_H

#include <VarNotifier.h>
#include "visionplugin.h"

class PluginStats;

class BotStats {
public:
    BotStats(){};
    float x, y;
    int team; // 0 = blue, 1 = yellow
    int id;
};

class FrameStats {
public:
    FrameStats(){};

    int frameid = -1;

    int robottotal = 0;
    int yellowbots = 0;
    int bluebots = 0;

    unordered_map<string, double> timings;
    unordered_map<int, BotStats> bots;

};

class Stats {
public:
    Stats(){};

    int framecount = 0;
    vector<FrameStats> frames;
    unordered_map<string, double> cumulative_timings;
    unordered_map<int, int> botcountsblue;
    unordered_map<int, int> botcountsyellow;

    void reset() {
        framecount = 0;
        frames.clear();
        cumulative_timings.clear();
        botcountsblue.clear();
        botcountsyellow.clear();
    }
};





class PluginStats : public VisionPlugin {
protected:
    VarList* _settings;
    VarNotifier _notifier;

    string title;

    bool enabled;
    bool written;

    Stats stats;
    int target_frames = 0;

public:
    PluginStats(FrameBuffer *_buffer);

    ProcessResult process(FrameData *data, RenderOptions *options) override;

    VarList *getSettings() override;
};


#endif //SSL_VISION_PLUGINSTATS_H
