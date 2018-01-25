// (c) Shahar Gino, January-2018, sgino209@gmail.com

#include "main.hpp"

#define Q_LEN     3
#define ESC_KEY  27

void usage(char* name) {
  cout << "usage: " << name << " <video file> [-debug]" << endl;
}

//-------------------------------------------------------------------------------------

objdet_t object_detection(deque<Mat> &in_frames, roi_t roi, pair<double,double> thresholds, opmode_t opmode, bool debug) {

  static bool init_done = false;
  objdet_t result;

  // Peek oldest frame from queue:
  Mat frame = in_frames.front();

  // PreProcess (common):
  Mat imgBlurred;
  GaussianBlur(frame, imgBlurred, Size(21,21), 0);

  // Object detection:
  switch (opmode) {

    case OPMODE_BS: 
    {
            static Mat fgMaskMOG2;
            static Ptr<BackgroundSubtractor> pMOG2;

            // Create Background Subtractor objects:
            if (!init_done) {
              pMOG2 = createBackgroundSubtractorMOG2(); //MOG2 approach
              namedWindow("BS_(fsMog2)");
            }
      
            // Update the background model:
            Mat cropped;
            double roiCx = roi.x + roi.w / 2.0;
            double roiCy = roi.y + roi.h / 2.0;
            getRectSubPix(imgBlurred, Size2f(float(roi.w), float(roi.h)), Point2f(float(roiCx), float(roiCy)), cropped);
            pMOG2->apply(cropped, fgMaskMOG2);
           
            // Metrics calculation:
            double metric_mean = mean(fgMaskMOG2).val[0];
            double metric_norm = norm(fgMaskMOG2) / fgMaskMOG2.total();

            double vel_x = 0.0; // BS does not have
            double vel_y = 0.0; // motion information  

            result.metrics.push_back( metric_mean ); 
            result.metrics.push_back( metric_norm );

            result.motion_vec = make_pair(vel_x,vel_y);

            result.triggering = (metric_mean > thresholds.first) && (metric_norm > thresholds.second); 

            // Display:
            if (debug) {
              imshow("BS_(fsMog2)", fgMaskMOG2);
            }
            break;
    }

    // -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. --
    
    case OPMODE_BS4: 
    {
            static Mat fgMaskMOG2_concat;
            static vector<Mat> fgMaskMOG2;
            static vector<Ptr<BackgroundSubtractor>> pMOG2;

            // Create Background Subtractor objects:
            if (!init_done) {
              fgMaskMOG2.resize(BS_DIRECTIONS);
              for (int i=0; i<BS_DIRECTIONS; i++) {
                pMOG2.push_back( createBackgroundSubtractorMOG2() ); //MOG2 approach
              }
              namedWindow("BS_(fsMog2)_LEFT");
              namedWindow("BS_(fsMog2)_RIGHT");
              namedWindow("BS_(fsMog2)_TOP");
              namedWindow("BS_(fsMog2)_BOTTOM");
              namedWindow("BS_(fsMog2)");
            }
      
            // Update the background model:
            vector<Mat> cropped;
            cropped.resize(BS_DIRECTIONS);
            double roiCx = roi.x + roi.w / 2.0;
            double roiCy = roi.y + roi.h / 2.0;
            getRectSubPix(imgBlurred, Size2f(float(roi.w/2), float(roi.h)),   Point2f(float(roiCx-roi.w/4), float(roiCy)),         cropped[BS_LEFT]);
            getRectSubPix(imgBlurred, Size2f(float(roi.w/2), float(roi.h)),   Point2f(float(roiCx+roi.w/4), float(roiCy)),         cropped[BS_RIGHT]);
            getRectSubPix(imgBlurred, Size2f(float(roi.w),   float(roi.h/2)), Point2f(float(roiCx),         float(roiCy-roi.h/4)), cropped[BS_TOP]);
            getRectSubPix(imgBlurred, Size2f(float(roi.w),   float(roi.h/2)), Point2f(float(roiCx),         float(roiCy+roi.h/4)), cropped[BS_BOTTOM]);
            for (int i=0; i<BS_DIRECTIONS; i++) {
              pMOG2[i]->apply(cropped[i], fgMaskMOG2[i]);
            }
              
            hconcat(fgMaskMOG2[BS_LEFT], fgMaskMOG2[BS_RIGHT], fgMaskMOG2_concat); // horizontal concatenation
         
            // Metrics calculation:
            double metric_mean = mean(fgMaskMOG2_concat).val[0];
            double metric_norm = norm(fgMaskMOG2_concat) / fgMaskMOG2_concat.total();
            
            vector<double> metric_mean_vec(BS_DIRECTIONS,0);
            vector<double> metric_norm_vec(BS_DIRECTIONS,0);
            vector<bool>   triggering_vec(BS_DIRECTIONS,false);
            for (int i=0; i<BS_DIRECTIONS; i++) {
              metric_mean_vec[i] = mean(fgMaskMOG2[i]).val[0];
              metric_norm_vec[i] = norm(fgMaskMOG2[i]) / fgMaskMOG2[i].total();
              triggering_vec[i] = (metric_mean_vec[i] > thresholds.first) && (metric_norm_vec[i] > thresholds.second); 
            }

            double vel_x = triggering_vec[BS_LEFT] ? 1 : triggering_vec[BS_RIGHT]  ? -1 : 0;
            double vel_y = triggering_vec[BS_TOP]  ? 1 : triggering_vec[BS_BOTTOM] ? -1 : 0;

            result.metrics.push_back( metric_mean ); 
            result.metrics.push_back( metric_norm );

            result.motion_vec = make_pair(vel_x,vel_y);

            result.triggering = (metric_mean > thresholds.first) && (metric_norm > thresholds.second); 

            // Display:
            if (debug) {
              imshow("BS_(fsMog2)_LEFT", fgMaskMOG2[BS_LEFT]);
              imshow("BS_(fsMog2)_RIGHT", fgMaskMOG2[BS_RIGHT]);
              imshow("BS_(fsMog2)_TOP", fgMaskMOG2[BS_TOP]);
              imshow("BS_(fsMog2)_BOTTOM", fgMaskMOG2[BS_BOTTOM]);
              imshow("BS_(fsMog2)", fgMaskMOG2_concat);
            }
            break;
    }
    
    // -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. --
    
    case OPMODE_LKT:

            // TBD...
            break;

    // -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. -- .. --

    default: 
            cout << "Unsupported operation mode: " << opmode << endl;
            break;
  }

  init_done = true;

  return result;
}

//-------------------------------------------------------------------------------------

int main(int argc, char** argv) {

  // User arguments:
  if ((argc < 2) || (strncmp(argv[1],"-h",2) == 0)) {
    usage(argv[0]);
    return 0;
  }
  std::string video_in(argv[1]); 
  bool debug = ((argc > 2) && (strncmp(argv[2],"-debug",6) == 0));

  // TODO: make the below user-arguments as well:
  opmode_t opmode = OPMODE_BS4; 
  roi_t roi = roi_t(100,400,300,400);
  pair<double,double> thresholds = make_pair(40.0,0.3);

  // Initialization:
  VideoCapture video_rd(video_in);
  if ( !video_rd.isOpened() ) {
    cout << "Cannot open the video file." << endl;
    return -1;
  }
  double fps  = video_rd.get(CV_CAP_PROP_FPS);
  int frame_w = video_rd.get(CV_CAP_PROP_FRAME_WIDTH); 
  int frame_h = video_rd.get(CV_CAP_PROP_FRAME_HEIGHT);   
  objdet_t result;
  Mat frame;
  deque<Mat> in_frames;
  namedWindow("Object_Detection", WINDOW_NORMAL); 
  VideoWriter video_wr("out.avi", CV_FOURCC('M','J','P','G'), fps, Size(frame_w,frame_h));

  // Read video frames + Processing:
  while(1) {

    // Read a new frame (+buffering):
    if ( !video_rd.read(frame) ) { 
      cout << "\n Cannot read the video file." << endl;
      break;
    }
    in_frames.push_back(frame);

    // Object Detection:
    if (in_frames.size() >= Q_LEN) {
      result = object_detection(in_frames, roi, thresholds, opmode, debug);
      
      // Release oldest frame:
      in_frames.pop_front();
    }
  
    // Results marking:
    ostringstream metric_tmp;
    string title = "Frame " + to_string(int(video_rd.get(CV_CAP_PROP_POS_FRAMES))) + ": ";
    for (double metric : result.metrics) {
      metric_tmp << metric << setprecision(2);
      title += metric_tmp.str() + ", ";
    }
    ostringstream vec_x, vec_y;
    vec_x << result.motion_vec.first << setprecision(2);
    vec_y << result.motion_vec.second << setprecision(2);
    title += "Motion=(" + vec_x.str() + "," + vec_y.str() + "), "; 
    title += "Trig= " + to_string(result.triggering); 
    putText(frame, title.c_str(), Point(10, 300),
            FONT_HERSHEY_SIMPLEX, 1.0 , Scalar(255,0,0), 2);

    // ROI marking:
    Scalar roi_color = result.triggering ? Scalar(0,0,255) : Scalar(255,0,0); 
    rectangle(frame, Rect(roi.x, roi.y, roi.w, roi.h), roi_color, 2);
    putText(frame, "ROI", Point(roi.x+roi.w/2-10, roi.y-10),
            FONT_HERSHEY_SIMPLEX, 1.0 , roi_color, 2);
    
    // Display result (+ write to file):
    imshow("Object_Detection", frame);
    video_wr.write(frame);

    // Exit with ESC:
    if (waitKey(1) == ESC_KEY) {
      break;
    }
  }

  // Epilog:
  video_rd.release();
  video_wr.release();
  destroyAllWindows();

  return 0;
}

