// (c) Shahar Gino, January-2018, sgino209@gmail.com

#ifndef MAIN_HPP
#define MAIN_HPP

#include <stdio.h>
#include <string.h>
#include <deque>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/video/video.hpp>

using namespace cv;
using namespace std;

enum {
  BS_LEFT = 0,
  BS_RIGHT,
  BS_TOP,
  BS_BOTTOM,
  BS_DIRECTIONS
};

typedef enum {
  OPMODE_BS = 0,  // Background Substruction (BS)
  OPMODE_BS4,     // 4 x Background Substruction (BS), for a rough motion extraction
  OPMODE_LKT,     // Optical Flow (LKT)
  OPMODE_TBD
} opmode_t; 

struct roi_st {
  unsigned int x;
  unsigned int y;
  unsigned int w;
  unsigned int h;
  roi_st(unsigned int x, unsigned int y, unsigned int w, unsigned int h): 
    x(x), y(y), w(w), h(h) {}
};
typedef struct roi_st roi_t;

struct objdet_st {
  vector<double> metrics;
  pair<double,double> motion_vec;
  bool triggering;
};
typedef struct objdet_st objdet_t;

void usage(char* name);

int main(int argc, char** argv);

objdet_t object_detection(deque<Mat> &in_frames, roi_t roi, pair<double,double> thresholds, opmode_t opmode);

#endif	// MAIN_HPP

