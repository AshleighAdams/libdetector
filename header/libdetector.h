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

    float GetDescriptor(motion_t* Motion);

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
        // Sets the Descriptor of an object to use (these normally use extra CPU!)
        void SetDescriptor(IDescriptor* Descriptor);
    protected:
    private:
        imagesize_t         m_sSize;
        CDetectorImage*     m_pLastImage;
        unsigned char       m_sDiffrenceThreshold;
        target_t*           m_pTargets[MAX_TARGETS];
        int                 m_iTargets;
        float               m_flMinTargSize;
        IDescriptor*        m_pDescriptor;
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
        void SetNewTargetThreshold(float flAmmount);
        void SetEvent(EventType type, void* function);
    private:
        float               m_flLastSeenLifeTime;
        float               m_flNewTargetThreshold;
        TrackedObjects      m_TrackedObjects;
        targetid            m_CurrentID;
        NewTargetFn         m_pNewTargEvent;
        UpdateTargetFn      m_pUpdateEvent;
        LostTargetFn        m_pLostTargEvent;
    };

    // Really simple, 3 density rings
    class CBaseDescriptor : public IDescriptor
    {
    public:
        CDescriptorValue* GetDescriptor(motion_t* Motion);
        char* GetName(CDescriptorValue* Descriptor, int Count);
    private:
    };
} // End Namespace

#endif // LIB_DET_H
