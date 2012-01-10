#ifndef DETECTOR_OPENCV_H
#define DETECTOR_OPENCV_H

// This is becuase we don't know their include paths or anything
#ifndef __OPENCV_CV_H__
#ERROR  OpenCV must be included before Detector
#endif

#include "libdetector.h"

#define XY_LOOP(_w_,_h_) for(int y = 0; y < _h_; y++) for(int x = 0; x < _w_; x++)
#define PMOTION_XY(_struct_, x, y) _struct_->motion[(x) + (y) * _struct_->size.width]

namespace Detector
{
    // From an OpenCV image to a Detector image
    void UpdateFrame(IplImage* From, CDetectorImage* To);
    // Detector image to OpenCV image
    void UpdateFrame(CDetectorImage* Img, IplImage* Frame);
    // Motion data to OpenCV image
    void UpdateFrame(motion_t* motion, IplImage* Frame);
}
#endif
