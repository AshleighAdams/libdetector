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
    void UpdateFrame(IplImage* From, CDetectorImage* To)
    {
        char* imgdata = From->imageData;
        int widthstep = From->widthStep;
        pixel_t* pix;

        XY_LOOP(From->width, From->height)
        {
            pix = To->Pixel(x,y);
            pix->b = ((unsigned char*)(imgdata + widthstep * y))[x*3];
            pix->g = ((unsigned char*)(imgdata + widthstep * y))[x*3+1];
            pix->r = ((unsigned char*)(imgdata + widthstep * y))[x*3+2];
        }
    }

    // Detector image to OpenCV image
    void UpdateFrame(CDetectorImage* Img, IplImage* Frame)
    {
        char* imgdata = Frame->imageData;
        int widthstep = Frame->widthStep;
        pixel_t* pix;

        XY_LOOP(Frame->width, Frame->height)
        {
            pix = Img->Pixel(x,y);
            ((unsigned char*)(imgdata + widthstep * y))[x*3] = pix->b;
            ((unsigned char*)(imgdata + widthstep * y))[x*3+1] = pix->g;
            ((unsigned char*)(imgdata + widthstep * y))[x*3+2] = pix->r;
        }
    }

    // Motion data to OpenCV image
    void UpdateFrame(motion_t* motion, IplImage* Frame)
    {
        char* imgdata = Frame->imageData;
        int widthstep = Frame->widthStep;

        unsigned char amm;
        XY_LOOP(Frame->width, Frame->height)
        {
            amm = (PMOTION_XY(motion, x, y)) ? 255 : 0;
            ((unsigned char*)(imgdata + widthstep * y))[x*3] = amm;
            ((unsigned char*)(imgdata + widthstep * y))[x*3+1] = amm;
            ((unsigned char*)(imgdata + widthstep * y))[x*3+2] = amm;
        }
    }
}
#endif
