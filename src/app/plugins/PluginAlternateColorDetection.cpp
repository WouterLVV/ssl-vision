//
// Created by wouter on 5/15/18.
//


#include "PluginAlternateColorDetection.h"
#include "cmvision_region.h"
//# define PI           3.14159265358979323846

PluginAlternateColorDetection::PluginAlternateColorDetection(FrameBuffer *_buffer, YUVLUT* lut) : VisionPlugin(_buffer) {
    _lut = lut;
}



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
int a = 255;

uchar probgaussianH(int a, int b, int c, int x) {
    double da = a, db = b, dc = c, dx = x;
    uchar res1 = (uchar)round(da*exp(-(pow(dx-db,2)/(2*pow(dc,2)))));
    uchar res2 = (uchar)round(da*exp(-(pow(dx-(db-180),2)/(2*pow(dc,2)))));
    uchar res3 = (uchar)round(da*exp(-(pow(dx-(db+180),2)/(2*pow(dc,2)))));
    return max(res1,max(res2,res3));
}

uchar probgaussianV(int a, int b, int c, int x) {
    double da = a, db = b, dc = c, dx = x;
    uchar res1 = (uchar)round(da*exp(-(pow(dx-db,2)/(2*pow(dc,2)))));
    return res1;
}

void buildHlut(uchar* lut) {
    for (int h = 0; h < 180; h++) {
        int j = 5*h;
        lut[j]  = probgaussianH(a, greencenter, greendev, h);
        lut[j+1] = probgaussianH(a, pinkcenter, pinkdev, h);
        lut[j+2] = probgaussianH(a, yellowcenter, yellowdev, h);
        lut[j+3] = probgaussianH(a, bluecenter, bluedev, h);
        lut[j+4] = max(lut[j], max(lut[j+1], max(lut[j+2], lut[j+3])));
    }
    lut[180*5] = 0;
    lut[180*5+1] = 0;
    lut[180*5+2] = 0;
    lut[180*5+3] = 0;
    lut[180*5+4] = 0;

}

//void testgaussian() {
//    uchar s = 255;
//    uchar v = 255;
//    for (uchar h = 0; h < 180; h += 3) {
//        p_dist converted;
//            converted.black  = 255-v;
//            converted.white  = (v*(255-s))/255;
//
//            fprintf(stdout, "%3d: d = %3d, w = %3d, g = %3d, p = %3d, y = %3d, b = %3d\n", h, converted.black, converted.white, converted.green, converted.purple, converted.yellow, converted.blue);
//
//    }
//}

int hor_regions = 3;
int vert_regions = 3;
int num_regions = hor_regions * vert_regions;
int avgradius = 3;

p_dist* PluginAlternateColorDetection::processHSV(cv::Mat img_hsv, uchar* hlut, p_dist* converted) {
    uchar* hsvdata = (img_hsv).data;
    int rows = (img_hsv).rows, cols = (img_hsv).cols, size = rows*cols;
    unsigned int hist[num_regions][256];
    unsigned int tmpavg[num_regions][256-2*avgradius];
    unsigned int avgv[num_regions];
    for (int i = 0; i < num_regions; i++) {
        for (int j = 0; j < 256; j++) {
            hist[i][j] = 0;
        }
    }

    int reg_width = cols/hor_regions + (cols%hor_regions == 0 ? 0 : 1);
    int reg_height = rows/vert_regions + (rows%vert_regions == 0 ? 0 : 1);

    //testgaussian();
    for (int i = 0; i < size*3; i+=3) {
        uchar v = hsvdata[i + 2];
        hist[(((i/3)/cols)/reg_height)*hor_regions+((i/3)%cols)/reg_width][v]++;
    }


    for (int i = 0; i < num_regions; i++) {


        // 5-point average
        int currentavg = 0;
        for (int j = 0; j < avgradius*2; j++) {
            currentavg += hist[i][j];
        }

        for (int j = 0+avgradius; j < 256-avgradius; j++) {
            currentavg += hist[i][j+avgradius];
            tmpavg[i][j-avgradius] = currentavg/(2*avgradius+1);
            currentavg -= hist[i][j-avgradius];
        }


        int highestindex = 0;
        for (int j = 1; j < 256-2*avgradius; j++) {
            if (tmpavg[i][j] > tmpavg[i][highestindex]) {
                highestindex = j;
            }
        }
        int j = highestindex;
        int stableradius = avgradius*2 +1;
        signed int lastpoints[stableradius];
        for (int k = 0; k < stableradius; k++){
            lastpoints[k] = -1000000;
        }
        for (; j < 256-(2*avgradius); j++) {
            int comparepoint = lastpoints[j%stableradius] = tmpavg[i][j];
            if (comparepoint > avgv[highestindex]/2) continue;
            bool isstable = true;

            for (int k = 0; k < stableradius; k++){
                isstable = isstable && abs(lastpoints[k] - comparepoint) < 8;
            }
            if (isstable) break;

        }

        //for ( ;j < 256-2*avgradius && tmpavg[i][j-1] + (sqrt(size/256)) >= tmpavg[i][j]; j++);
        //for ( ;j < 256-2*avgradius && tmpavg[i][j-1] + (sqrt(size/256)) >= tmpavg[i][j]; j++);

        avgv[i] = j + avgradius;

//        hist[i] /= size/num_regions;
//        hist[i] *= 0.95;
//        hist[i] += 255*0.05;
    }

    int pixelcount = 0;
    for (int i = 0; i < size*3; i+=3) {
        uchar h = hsvdata[i];
        uchar s = hsvdata[i + 1];
        uchar v = hsvdata[i + 2];

        int j = i/3, hi = h*5;

        converted[j].h = h;
        converted[j].s = s;
        converted[j].v = v;
//        if (v < hist) {
//        if (v < 20) {
//            converted[j].used = true;
//            converted[j].black = 255;
//            converted[j].white = 0;
//            converted[j].colors = 0;
//            continue;
//
//        }

        converted[j].colors = hlut+hi;
        converted[j].vavg = avgv[(((i/3)/cols)/reg_height)*hor_regions+((i/3)%cols)/reg_width];
        converted[j].used = false; //converted[j].colors[4] < 100 || converted[j].v < converted[j].vavg;
        if (converted[j].used) continue;
        converted[j].black = probgaussianV(a, blackcenter, blackdev, v);
        converted[j].white = 255-s;


//        converted[j].green = hlut[hi];
//        converted[j].purple = hlut[hi+1];
//        converted[j].yellow = hlut[hi+2];
//        converted[j].blue = hlut[hi+3];
        //fprintf(stdout, "%3dx%3d: d = %3d, w = %3d, g = %3d, p = %3d, y = %3d, b = %3d\n", j%cols, j/cols, converted[j].black, converted[j].white, converted[j].colors[0], converted[j].colors[1], converted[j].colors[2], converted[j].colors[3]);
        pixelcount++;
    }
    fprintf(stdout, "counted %d pixels\n", pixelcount);
    return converted;
}

uchar maxHdist = 30;
uchar maxVdist = 50;

blob makeblob(p_dist converted[], int pixel, int cols, int size) {
    blob result;
    unsigned long totalv = converted[pixel].v;
    double hsin = sin(((double)converted[pixel].h)*M_PI/90), hcos = cos(((double)converted[pixel].h)*M_PI/90);
    unsigned int pixelcount = 1, failcount = 0;
    queue<int> q;
    q.push(pixel);
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        p_dist* cp = converted+current;

        if (cp->used)  {
            //failcount++;
            continue;
        }

        signed int avgh = atan2(hsin, hcos)*90/M_PI;
        signed int avgv = totalv/pixelcount;
        if (avgh < 0) avgh += 180;
        //fprintf(stdout, "avgh: %d\n", avgh);

        if (cp->s != 0) {
            if (min(abs(avgh - (signed int)cp->h), abs(avgh - (signed int)cp->h + (cp->h < 90 ? 180 : -180))) > maxHdist || abs(avgv - (signed int)cp->v) > maxVdist)  {
                //failcount++;
                continue;
            }

            hsin += sin(((double)cp->h)*M_PI/90);
            hcos += cos(((double)cp->h)*M_PI/90);
            totalv += cp->v;
            pixelcount++;
        }
        result.pixels.push_back(current);
        cp->used = true;
        result.size++;
        if (current%cols != 0 && !converted[current-1].used) {
            q.push(current-1);
        }
        if (current%cols != cols-1 && !converted[current+1].used) {
            q.push(current+1);
        }
        if (current > cols) {
            q.push(current-cols && !converted[current-cols].used);
        }
        if (current + cols < size && !converted[current+cols].used) {
            q.push(current+cols);
        }

    }
//    if ((double)pixelcount / (double)failcount > 0.8) {
//        result.size = 0;
//    }

    return result;
}



int minpixels = 20;
int maxpixels = 300;

void blobdetection(p_dist* converted, int cols, int size, vector<blob> * blobs) {
    //vector<blob> blobs;
    for (int i = 0; i < size; i++) {
        if (converted[i].colors != 0 && !converted[i].used) {
            blob b = makeblob(converted, i, cols, size);
            if (b.size > minpixels && b.size < maxpixels) {


                unsigned long green = 0, purple = 0, yellow = 0, blue = 0;
                b.max_x = 0;
                b.min_x = cols;
                b.max_y = 0;
                b.min_y = size/cols;
                b.cen_x = 0;
                b.cen_y = 0;

                for (const auto &px : b.pixels) {
                    int x = px%cols, y = px/cols;
                    if (x > b.max_x) b.max_x = x;
                    if (x < b.min_x) b.min_x = x;
                    if (y > b.max_y) b.max_y = y;
                    if (y < b.min_y) b.min_y = y;
                    b.cen_x += x;
                    b.cen_y += y;
                }

                b.cen_x /= b.size;
                b.cen_y /= b.size;

                double ratio = ((double)(b.max_x - b.min_x + 1)) / ((double)(b.max_y - b.min_y + 1));
                if (ratio < 1.0) ratio = 1.0/ratio;
                if (ratio > 3) continue;

                int area = (b.max_x - b.min_x + 1) * (b.max_y - b.min_y + 1);
                double efficiency = ((double)b.size/(double)area);
                if (efficiency < 0.4) continue;

                for (int k = b.min_y; k <= b.max_y; k++) {
                    for (int l = b.min_x; l <= b.min_x; l++) {
                        int px = k * cols + l;
                        if (converted[px].s > 0 && converted[px].v > converted[px].vavg*0.6) {
                            green  += converted[px].colors[0] * pow(converted[px].s, 2);
                            purple += converted[px].colors[1] * pow(converted[px].s, 2);
                            yellow += converted[px].colors[2] * pow(converted[px].s, 2);
                            blue   += converted[px].colors[3] * pow(converted[px].s, 2);
                        }
                    }
                }


                if (green >= purple && green >= yellow && green >= blue ) {
                    b.color = 1;
                } else if (purple >= yellow && purple >= blue) {
                    b.color = 2;
                } else if (yellow >= blue) {
                    b.color = 3;
                } else {
                    b.color = 4;
                }

                blobs->push_back(b);
            }
        }
    }
    fprintf(stdout, "blobs: %lu\n", blobs->size());
    fflush(stdout);

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

    uchar* hlut;
    if ((hlut = (uchar*)data->map.get("acd_hlut")) == 0) {
        hlut = new uchar[5*181];
        buildHlut(hlut);
        data->map.insert("acd_hlut", hlut);
    }

    p_dist* converted;
    if ((converted = (p_dist*)data->map.get("acd_converted")) == 0) {
        converted = new p_dist[data->video.getNumPixels()];
        data->map.insert("acd_converted", converted);
    }

    vector<blob> * blobs;
    if ((blobs = (vector<blob>*)data->map.get("acd_blobs")) == 0) {
        blobs = new vector<blob>;
        data->map.insert("acd_blobs", blobs);
    }


    CMVision::RegionList * reglist;
    if ((reglist=(CMVision::RegionList *)data->map.get("cmv_reglist")) == 0) {
        reglist=(CMVision::RegionList *)data->map.insert("cmv_reglist",new CMVision::RegionList(1000));
    }

//    CMVision::ColorRegionList * crl;
//    if ((crl = (CMVision::ColorRegionList*)data->map.get("acd_colorlist")) == 0) {
//        crl = new CMVision::ColorRegionList(_lut->getChannelCount());
//        data->map.insert("acd_colorlist", crl);
//    }

    CMVision::ColorRegionList * colorlist;
    if ((colorlist=(CMVision::ColorRegionList *)data->map.get("cmv_colorlist")) == 0) {
        colorlist=(CMVision::ColorRegionList *)data->map.insert("cmv_colorlist",new CMVision::ColorRegionList(_lut->getChannelCount()));
    }



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
        //uchar hlut[4*180];
        int rows = (img_hsv).rows, cols = (img_hsv).cols, size = rows*cols;
        processHSV(img_hsv, hlut, converted);
        blobs->clear();
        blobdetection(converted, cols, size, blobs);


        raw8 color_id_yellow = _lut->getChannelID("Yellow");
        if (color_id_yellow == -1) printf("WARNING color label 'Yellow' not defined in LUT!!!\n");
        raw8 color_id_blue = _lut->getChannelID("Blue");
        if (color_id_blue == -1) printf("WARNING color label 'Blue' not defined in LUT!!!\n");
        raw8 color_id_green = _lut->getChannelID("Green");
        if (color_id_green == -1) printf("WARNING color label 'Green' not defined in LUT!!!\n");
        raw8 color_id_pink = _lut->getChannelID("Pink");
        if (color_id_pink == -1) printf("WARNING color label 'Pink' not defined in LUT!!!\n");

        raw8 color_id_clear = 0;

        raw8 color_id_ball = _lut->getChannelID("Orange");
        if (color_id_ball == -1) printf("WARNING color label 'Orange' not defined in LUT!!!\n");

        raw8 color_id_black = _lut->getChannelID("Black");
        if (color_id_black == -1) printf("WARNING color label 'Black' not defined in LUT!!!\n");

        raw8 color_id_field = _lut->getChannelID("Field Green");
        if (color_id_field == -1) printf("WARNING color label 'Field Green' not defined in LUT!!!\n");
        int i = 0;
        //CMVision::RegionList reglist(1000);
        reglist->setUsedRegions(blobs->size());
        for (const auto & blob : *blobs) {

            CMVision::Region * reg = reglist->getRegionArrayPointer();
            raw8 color = -1;
            switch (blob.color) {
                case 1:
                    color = color_id_green;
                    break;
                case 2:
                    color = color_id_pink;
                    break;
                case 3:
                    color = color_id_yellow;
                    break;
                case 4:
                    color = color_id_blue;
                    break;
            }
            reg[i].color = color;
            reg[i].x1 = blob.min_x;
            reg[i].x2 = blob.max_x;
            reg[i].y1 = blob.min_y;
            reg[i].y2 = blob.max_y;
            reg[i].cen_x = blob.cen_x;
            reg[i].cen_y = blob.cen_y;
            reg[i].area = blob.size;
            reg[i].iterator_id = 0;
            reg[i].next = 0;


            i++;
        }

        int max_area = CMVision::RegionProcessing::separateRegions(colorlist, reglist, minpixels);
        CMVision::RegionProcessing::sortRegions(colorlist,max_area);


//        for (int k = 0; k < colorlist->getNumColorRegions(); k++) {
//            fprintf(stdout, "Channel %d has %d entries.\n", k, colorlist->getColorRegionArrayPointer()[i].getNumRegions());
//            CMVision::Region * reg = colorlist->getColorRegionArrayPointer()[i].getInitialElement();
//            while (reg != 0) {
//                fprintf(stdout, "x: %f, y: %f\n", reg->cen_x, reg->cen_y);
//            }
//            fprintf(stdout, "\n");
//            fflush(stdout);
//        }

        return ProcessingOk;
    } else {
        fprintf(stderr,"This shit needs RGB8 as input image, but found: %s\n",Colors::colorFormatToString(data->video.getColorFormat()).c_str());
        return ProcessingFailed;
    }
}