#define COMPILE_OPENCV_MODULE
#ifdef COMPILE_OPENCV_MODULE

// STL includes
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>
#include <list>
#include <cmath>

// OpenCV includes
using namespace std; // OpenCV needs this else we get 500 errors....
#include "opencv/cv.h"
#include "opencv/highgui.h"

#define DETECTOR_OPENCV
#include "libdetector/include/libdetector.h"

void Detector::UpdateFrame(IplImage* From, CDetectorImage* To)
{
	char* imgdata = From->imageData;
	int widthstep = From->widthStep;
	pixel_t* pix;

	XY_LOOP(From->width, From->height)
	{
		pix = To->Pixel(x,y);
		if(pix)
		{
			pix->b = ((unsigned char*)(imgdata + widthstep * y))[x*3];
			pix->g = ((unsigned char*)(imgdata + widthstep * y))[x*3+1];
			pix->r = ((unsigned char*)(imgdata + widthstep * y))[x*3+2];
		}
	}
}

// Detector image to OpenCV image
void Detector::UpdateFrame(CDetectorImage* Img, IplImage* Frame)
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
void Detector::UpdateFrame(motion_t* motion, IplImage* Frame)
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

#endif
