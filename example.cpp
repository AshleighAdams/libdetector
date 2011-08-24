// STL includes
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>
#include <list>

// OpenCV includes
using namespace std; // OpenCV needs this else we get 500 errors....
#include "opencv/cv.h"
#include "opencv/highgui.h"

// Detector includes (must be after OpenCV)
#define DETECTOR_OPENCV
#include "libdetector/include/libdetector.h"

using namespace Detector;

void NewObject(CTrackedObject* Obj)
{
    cout << "Tracking new object! @ " << GetCurrentTime() << "\n";
}

void LastObject(CTrackedObject* Obj)
{
    cout << "Lost object! @ " << GetCurrentTime() << "\n";
}

CDetector* g_pDetector = NULL;
CObjectTracker* g_pTracker = NULL;

// Settings n stuff
bool g_bWriteFrame = false;
bool g_bExiting = false;

void SettingsThread()
{
    char input[255];
    cout << (g_bWriteFrame ? "Saving on" : "Saving off") << "\n";
    while(!g_bExiting)
    {
        cin >> input;

        if(strcmp(input, "togglerecord") == 0)
        {
            g_bWriteFrame = !g_bWriteFrame;
            cout << (g_bWriteFrame ? "Saving on" : "Saving off") << "\n";
        }
        else if(strcmp(input, "sensetivity") == 0)
        {
            cout << "Enter ammount: \n";
            short amm;
            cin >> amm;
            g_pDetector->SetDiffrenceThreshold(amm);
        }else if(strcmp(input, "exit") == 0)
        {
            g_bExiting = true;
            return;
        }
        else
            cout << "Setting not found\n";
    }
}

target_t* g_Targets[MAX_TARGETS];
int g_Count = 0;

bool ProccessFrame(CDetectorImage* Image)
{
    if(!g_pDetector && !g_pTracker)
    {
        g_pDetector = new CDetector(Image->GetSize());

        //CBaseDescriptor* Disc = new CBaseDescriptor();
        //g_pDetector->SetDescriptor(Disc);

        g_pTracker = new CObjectTracker();
        g_pTracker->SetEvent(EVENT_NEWTARG,    (void*)&NewObject);
        g_pTracker->SetEvent(EVENT_LOST,       (void*)&LastObject);
    }
    g_pDetector->PushImage(Image);
    g_Count = g_pDetector->GetTargets(g_Targets);

    g_pTracker->PushTargets(g_Targets, g_Count);

    TrackedObjects Objs = *g_pTracker->GetTrackedObjects();

    color_t NoneTrackedCol;
    color_t TrackedCol;

    NoneTrackedCol.r = 255;
    NoneTrackedCol.g = 0;
    NoneTrackedCol.b = 0;

    TrackedCol.r = 0;
    TrackedCol.g = 255;
    TrackedCol.b = 0;

    Image->DrawColor(NoneTrackedCol);
    for(int i = 0; i < g_Count; i++)
        Image->DrawTarget(g_Targets[i]);

    bool ret = false;
    Image->DrawColor(TrackedCol);

    for(CTrackedObject* Obj : Objs)
    {
        ret = true;
        Image->DrawTarget(Obj);
        //DrawTarget(Image, Obj);
    }
    return ret;
}

CDetectorImage* g_pDetectorImage;

int main()
{
    thread thrd(SettingsThread);

    CvCapture* capture = cvCaptureFromCAM( 0 );

    if ( !capture )
    {
        fprintf( stderr, "ERROR: capture is NULL \n" );
        return 1;
    }

    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 260);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FOURCC, CV_FOURCC('I', 'Y', 'U', 'V'));

    IplImage* frame = cvQueryFrame( capture );
    if ( !frame )
    {
        fprintf( stderr, "ERROR: frame is null...\n" );
        return 1;
    }

    // let the cam initiate
    double sleeptime = GetCurrentTime();
    while(sleeptime + 5 > GetCurrentTime()) frame = cvQueryFrame( capture );

    CvSize imgSize;
    imgSize.width = frame->width;
    imgSize.height = frame->height;

    double fps = 10.0;
    CvVideoWriter *writer = cvCreateVideoWriter(
            "out.avi",
            CV_FOURCC('M', 'J', 'P', 'G'),
            fps,
            imgSize
            );


    // Create a window in which the captured images will be presented
    cvNamedWindow( "Detector", CV_WINDOW_AUTOSIZE );
    cvNamedWindow( "Detector - Motion", CV_WINDOW_AUTOSIZE );
    cvMoveWindow( "Detector - Motion", 640, 0);
    // Show the image captured from the camera in the window and repeat
    while ( !g_bExiting )
    {
        // Get one frame
        frame = cvQueryFrame( capture );
        if ( !frame )
        {
            fprintf( stderr, "ERROR: frame is null...\n" );
            break;
        }

        if(!g_pDetectorImage)
            g_pDetectorImage = new CDetectorImage(frame->width, frame->height);
        UpdateFrame(frame, g_pDetectorImage);

        bool WriteFrame = ProccessFrame(g_pDetectorImage);
        UpdateFrame(g_pDetectorImage, frame);

        if(WriteFrame && g_bWriteFrame)
            cvWriteFrame(writer, frame);

        cvShowImage( "Detector", frame );

        if(g_pDetector->GetMotionImage())
        {
            UpdateFrame(g_pDetector->GetMotionImage(), frame);
            cvShowImage( "Detector - Motion", frame );
        }
        // Enter breaks loop
        if ( (cvWaitKey(10) & 255) == 27 ) break;
    }
    // Release the capture device housekeeping
    cvReleaseCapture( &capture ); // Likes to segfault
    cvDestroyWindow( "Detector" );
    cvDestroyWindow( "Detector - Motion");
    cvReleaseVideoWriter(&writer);

    return 0;
}
