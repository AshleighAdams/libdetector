#ifndef LIB_DET_H
#define LIB_DET_H

#define _DET_INTERFACE_LOAD
#include "interface.h"

namespace Detector
{
    class CDetector : public IDetector
    {
        friend motion_t* AbsoluteDiffrence( CDetector* self, CDetectorImage* img1, CDetectorImage* img2 );
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
    protected:
    private:
        imagesize_t         m_sSize;
        CDetectorImage*     m_pLastImage;
        unsigned char       m_sDiffrenceThreshold;
        target_t*           m_pTargets[MAX_TARGETS];
        int                 m_iTargets;
        float               m_flMinTargSize;
    };

    motion_t* AbsoluteDiffrence( CDetector* self, CDetectorImage* img1, CDetectorImage* img2 );

    class CObjectTracker : public IObjectTracker
    {
    public:
        CObjectTracker();
        ~CObjectTracker();
        void PushTargets( target_t* Targets[MAX_TARGETS], int Count );
        TrackedObjects* GetTrackedObjects();
        void SetLastSeenLifeTime(float flAmmount); // The target will still exist and simulate if the target goes out of view
    private:
        float               m_flLastSeenLifeTime;
        TrackedObjects      m_TrackedObjects;
        int m_CurrentID;
    };
} // End Namespace

#endif // LIB_DET_H
