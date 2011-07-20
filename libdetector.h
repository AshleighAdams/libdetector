#ifndef LIB_DET_H
#define LIB_DET_H

#include "interface.h"

class CDetector : public IDetector
{
    friend motion_t* AbsoluteDiffrence(CDetector* self, CDetectorImage* img1, CDetectorImage* img2);
public:
    CDetector(imagesize_t Size);
    ~CDetector();
    // Push the next image here, calculates the new target
    bool PushImage(CDetectorImage *Image);
    // Returns the number of targets, outputs to argument
    int GetTargets(target_t* Targets[MAX_TARGETS]);
    int GetNumberOfTargets();
    void SetDiffrenceThreshold(short sAmmount);
protected:
private:
    imagesize_t         m_sSize;
    CDetectorImage*     m_pLastImage;
    unsigned char       m_sDiffrenceThreshold;
    target_t*           m_pTargets[MAX_TARGETS];
    int                 m_iTargets;
};

motion_t* AbsoluteDiffrence(CDetector* self, CDetectorImage* img1, CDetectorImage* img2);

#endif // LIB_DET_H
