#ifndef LIB_DET_H
#define LIB_DET_H

#include "interface.h"

void AbsoluteDiffrence(CDetectorImage* img1, CDetectorImage* img2, motion_t* motion);

class CDetector : public IDetector
{
    friend void AbsoluteDiffrence(CDetectorImage* img1, CDetectorImage* img2, motion_t* motion);
public:
    CDetector(imagesize_t Size);
    ~CDetector();
    // Push the next image here, calculates the new target
    void PushImage(CDetectorImage *Image);
    // Returns the number of targets, outputs to argument
    int GetTargets(target_t* Targets[MAX_TARGETS]);
    void SetDiffrenceThreshold(short sAmmount);
protected:
private:
    imagesize_t         m_sSize;
    CDetectorImage*     m_pLastImage;
    unsigned char       m_sDiffrenceThreshold;
    target_t*           m_pTargets[MAX_TARGETS];
};

#endif // LIB_DET_H
