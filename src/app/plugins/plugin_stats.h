//
// Created by wouter on 4/26/18.
//

#ifndef SSL_VISION_PLUGINSTATS_H
#define SSL_VISION_PLUGINSTATS_H

#include "visionplugin.h"

class PluginStats;

class PosRotId {
public:
    PosRotId(int id, double x, double y, double theta) {
        this->id    = id;
        this->x     = x;
        this->y     = y;
        this->theta = theta;
    }

    int    getID()      { return id;    };
    double getX()       { return x;     };
    double getY()       { return y;     };
    double getTheta()   { return theta; };
    bool   isValid()    { return valid; };


    void   setValid(bool b) { valid = b; };

private:
    bool valid;
    int id;
    double x,y,theta;
};

class RobotHistoryElement {
public:
    int robotId;
    int seenTotal;
    std::vector<PosRotId> posrot;

};


class PluginStats : public VisionPlugin {
protected:
    std::vector<RobotHistoryElement> bluestats;
    std::vector<RobotHistoryElement> yellowstats;
public:
    PluginStats(FrameBuffer *_buffer);


    ProcessResult process(FrameData *data, RenderOptions *options) override;
};


#endif //SSL_VISION_PLUGINSTATS_H
