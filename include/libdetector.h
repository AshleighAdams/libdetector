#ifndef LIB_DET_H
#define LIB_DET_H

#define _DET_INTERFACE_LOAD
#include "interface.h"

namespace Detector
{
    const int PIXEL_NOMOTION = 0;
    const int PIXEL_MOTION = 1;
    const int PIXEL_SCANNEDMOTION = 2;

    typedef int EventType;
    typedef void(*NewTargetFn)(CTrackedObject* Obj);
    const EventType EVENT_NEWTARG = 0;

    typedef void(*UpdateTargetFn)(CTrackedObject* Obj, bool Simulated);
    const EventType EVENT_UPDATE = 1;

    typedef void(*LostTargetFn)(CTrackedObject* Obj);
    const EventType EVENT_LOST = 2;

    class CDetectorImage : public CDetectorBaseClass
    {
    public:
        CDetectorImage( int Width, int Height );
        CDetectorImage( imagesize_t size );
        ~CDetectorImage();
        CDetectorImage* Exclusive();
        pixel_t* Pixel( int x, int y );
        imagesize_t GetSize();
        // Now for some usefull functions
        void DrawColor(color_t Col);
        void DrawBox(position_t a, position_t b);
        void DrawTarget(CTrackedObject* pObj);
        void DrawTarget(target_t* pTarget);
        void DrawLine(position_t a, position_t b);
    private:
        imagesize_t m_sSize;
        pixel_t* m_psPixels;
        color_t m_DrawColor;
    };

    class CDetector : public IDetector
    {
        friend void Detector::AbsoluteDiffrence( CDetector* self, CDetectorImage* img1, CDetectorImage* img2, motion_t* Target );
    public:
        CDetector( imagesize_t Size );
        ~CDetector();
        // Push the next image here, calculates the new target
        bool PushImage( CDetectorImage* Image );
        // Returns the the number of targets, targets output to argument
        int GetTargets( target_t* Targets[MAX_TARGETS] );
        // Returns the number of targets
        int GetNumberOfTargets();
        // The absolute diffrence from pixels before motion is registerd
        void SetDiffrenceThreshold( short sAmmount );
        // Any targets smaller than this will be removed
        void SetMinTargSize( float flAmmount );
        // Sets the Descriptor of an object to use (these normally use extra CPU!)
        void SetDescriptor(IDescriptor* Descriptor);
        // Set motion blur ammount
        void SetMotionBlur(float flAmmount);

        // TODO: Below needs implenting

        // Returns the number of targets that are too small and removed
        unsigned int GetFalsePosCount();
        // Gets the total motion (0 being none, 1 being all)
        float GetTotalMotion();
        // Gets the motion image
        motion_t* GetMotionImage();
        // Sets the ignore image, ignore motion in these regions
        void SetIgnoreImage(CDetectorImage* Image);
    protected:
    private:
        imagesize_t         m_sSize;                // Size of the supplied images
        CDetectorImage*     m_pRefrenceImage;       // This is the image used to compare to
        motion_t*           m_pMotionImage;         // Usefull for testing
        unsigned char       m_sDiffrenceThreshold;  // If it goes above this, it is registerd as motion
        target_t*           m_pTargets[MAX_TARGETS];
        int                 m_iTargets;             // Number of targets
        int                 m_iFalsePos;            // Number of targets removed
        float               m_flMinTargSize;        // If a target is smaller than this, it's removed and m_iFalsePos is increased
        IDescriptor*        m_pDescriptor;          // The class that will identify what a target is
        float               m_flBlurAmmount;        // How much to blur the image per update (Motion blur on the background)
        float               m_flBlurMaxChange;      // Max it's allowed to change (Stop contrasting targets leaving a hige trail behind)
        int                 m_BlurUpdateRate;       // How many frames to do a background update
        int                 m_BlurUpdateRateFrame;  // Current frame (from 0 to m_BlurUpdateRate)
        bool                m_bCleverBackground;    // Background that doesn't update unless there is noise
        float               m_flTotalMotion;        // Total motion in the image.
        color_t*            m_pMotionPixel;         // \/
        CDetectorImage*     m_pIgnoreMotionImage;   // TODO: . Checks this image, and if any pixels are m_pMotionPixel, they are removed from the motion image
    };

    // Finds the absolute diffrence between 2 images.
    motion_t* AbsoluteDiffrence( CDetector* self, CDetectorImage* img1, CDetectorImage* img2 );

    class CObjectTracker : public IObjectTracker
    {
    public:
        CObjectTracker();
        ~CObjectTracker();
        // Update the targets
        void PushTargets( target_t* Targets[MAX_TARGETS], int Count );
        TrackedObjects* GetTrackedObjects();
        // The target will still exist and simulate if the target goes out of view
        void SetLastSeenLifeTime(float flAmmount);
        // Sets new target threshold
        void SetNewTargetThreshold(float flAmmount);
        // Self explanitary
        void SetEvent(EventType type, void* function);
    private:
        float               m_flLastSeenLifeTime;       // Remove a target if we havn't seen them for this long
        float               m_flNewTargetThreshold;     // Rather than the target being asigned to the wrong object, it instead creates a new one
        float               m_flNewTargetTimeThreshold; // A new target will be created if a target has gone untracked for this ammount of time
        float               m_flNewTargetTime;          // The actual time that is being counted
        TrackedObjects      m_TrackedObjects;           // ...
        targetid            m_CurrentID;                // This is increased and asigned to new targets
        NewTargetFn         m_pNewTargEvent;            // /*
        UpdateTargetFn      m_pUpdateEvent;             //      Event func ptrs
        LostTargetFn        m_pLostTargEvent;           // */
    };

    // Really simple, 3 density rings
    class CBaseDescriptor : public IDescriptor
    {
    public:
        CDescriptorValue* GetDescriptor(motion_t* Motion);
        char* GetName(CDescriptorValue* Descriptor, int Count);
        bool LoadDescriptor(char* File); // TODO: Implent this
    private:
    };

} // End Namespace


// If OpenCV exists, lets provide some helper stuff
#ifdef DETECTOR_OPENCV
#include "opencvhelper.h"
#endif

#endif // LIB_DET_H
