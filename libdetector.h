#ifndef LIB_DET_H
#define LIB_DET_H

#include "interface.h"

class CDetector : public IDetector
{
public:
    CDetector(imagesize_t Size);
    ~CDetector();
    // Push the next image here, calculates the new target
    void PushImage(CDetectorImage *Image);
    // Returns the number of targets, outputs to argument
    int GetTargets(target_t* Targets[MAX_TARGETS]);
    void SetDiffrenceThreshold(float flAmmount);
protected:
private:
    imagesize_t         m_sSize;
    CDetectorImage*     m_pLastImage;
    float               m_flDiffrenceThreshold;
};

#endif // LIB_DET_H
