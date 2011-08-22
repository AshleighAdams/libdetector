#include <iostream>
#include <thread>

#include "libdetector/header/libdetector.h"

#define XY_LOOP(_w_,_h_) for(int y = 0; y < _h_; y++) for(int x = 0; x < _w_; x++)
#define PMOTION_XY(_struct_, x, y) _struct_->motion[(x) + (y) * _struct_->size.width]

using namespace std;
using namespace Detector;

#include "opencv/cv.h"
#include "opencv/highgui.h"


CDetectorImage* Ret = NULL;
CDetectorImage* GetDetectorImage(IplImage* Frame)
{
    if(!Ret)
    {
        imagesize_t size;
        size.height = Frame->height;
        size.width = Frame->width;
        Ret = new CDetectorImage(size);
        cout << "Created new image.\n";
        cout << Frame->width << " :: " << Frame->height << "\n";
    }

    char* imgdata = Frame->imageData;
    int widthstep = Frame->widthStep;
    pixel_t* pix;


    XY_LOOP(Frame->width, Frame->height)
    {
        pix = Ret->Pixel(x,y);
        pix->b = ((unsigned char*)(imgdata + widthstep * y))[x*3];
        pix->g = ((unsigned char*)(imgdata + widthstep * y))[x*3+1];
        pix->r = ((unsigned char*)(imgdata + widthstep * y))[x*3+2];
    }
    return Ret;
}

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

void NewObject(CTrackedObject* Obj)
{
    cout << "Tracking new object! @ " << GetCurrentTime() << "\n";
}

void LastObject(CTrackedObject* Obj)
{
    cout << "Lost object! @ " << GetCurrentTime() << "\n";
}


CDetector* detector = NULL;
CObjectTracker* tracker = NULL;


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
            detector->SetDiffrenceThreshold(amm);
        }else if(strcmp(input, "exit") == 0)
        {
            g_bExiting = true;
            return;
        }
        else
            cout << "Setting not found\n";

    }
}

target_t* Targets[MAX_TARGETS];
int Count = 0;

bool ProccessFrame(CDetectorImage* Image)
{
    if(!detector && !tracker)
    {
        detector = new CDetector(Image->GetSize());

        CBaseDescriptor* Disc = new CBaseDescriptor();
        //detector->SetDescriptor(Disc);

        tracker = new CObjectTracker();
        tracker->SetEvent(EVENT_NEWTARG,    (void*)&NewObject);
        tracker->SetEvent(EVENT_LOST,       (void*)&LastObject);
    }
    detector->PushImage(Image);
    Count = detector->GetTargets(Targets);

    tracker->PushTargets(Targets, Count);

    TrackedObjects Objs = *tracker->GetTrackedObjects();

    for(int i = 0; i < Count; i++)
    {
        target_t* Targ = Targets[i];
        DrawTarget(Image, Targ);
    }
    bool ret = false;
    for(CTrackedObject* Obj : Objs)
    {
        ret = true;
        DrawTarget(Image, Obj);
    }
    return ret;
}

int main()
{
    std::thread thrd(SettingsThread);

    CvCapture* capture = cvCaptureFromCAM( 1 );

    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 260);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FOURCC, CV_FOURCC('I', 'Y', 'U', 'V'));


    if ( !capture )
    {
        fprintf( stderr, "ERROR: capture is NULL \n" );
        return -1;
    }

    IplImage* frame = cvQueryFrame( capture );
    if ( !frame )
    {
        fprintf( stderr, "ERROR: frame is null...\n" );
        return 0;
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
            getchar();
            break;
        }


        CDetectorImage* img = GetDetectorImage(frame);
        bool WriteFrame = ProccessFrame(img);
        UpdateFrame(img, frame);

        if(WriteFrame && g_bWriteFrame)
            cvWriteFrame(writer, frame);

        cvShowImage( "Detector", frame );

        if(detector->m_pMotionImage)
        {
            UpdateFrame(detector->m_pMotionImage, frame);
            cvShowImage( "Detector - Motion", frame );
        }
        // Do not release the frame!
        //If ESC key pressed, Key=0x10001B under OpenCV 0.9.7(linux version),
        //remove higher bits using AND operator
        if ( (cvWaitKey(10) & 255) == 27 ) break;
    }
    // Release the capture device housekeeping
    cvReleaseCapture( &capture ); // Likes to segfault
    cvDestroyWindow( "Detector" );
    cvDestroyWindow( "Detector - Motion");
    cvReleaseVideoWriter(&writer);

    return 0;
}
