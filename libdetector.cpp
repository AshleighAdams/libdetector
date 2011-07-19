
#include "libdetector.h"

// Helper crap to save time
#define MOTION_XY(_struct_, x, y) _struct_.motion[(x) + (y) * _struct_.size.width]
#define PMOTION_XY(_struct_, x, y) _struct_->motion[(x) + (y) * _struct_->size.width]

// Provides a byte for each pixel
struct motion_t
{
	imagesize_t size;
	unsigned char* motion;
};

struct motionhelper_t
{
    imagesize_t size;
    int MinX;
    int MinY;
    int MaxX;
    int MaxY;
    unsigned char* motion;
};


void DoNextScanLine(int x, int y, motionhelper_t* motion)
{
    if( x >= motion->size.width || y >= motion->size.height )
        return;
    if( PMOTION_XY(motion, x, y) == 0 )
        return;

    if (x > motion->MaxX)
        motion->MaxX = x;
    if (x < motion->MinX)
        motion->MinX = x;
    if (y > motion->MaxY)
        motion->MaxY = y;
    if (y < motion->MinY)
        motion->MinY = y;

    int y1 = y;

    //draw current scanline from start position to the top
    while (y1 < motion->size.height && PMOTION_XY(motion, x, y1) == 1)
    {
        PMOTION_XY(motion, x, y1) = 2;
        if (y1 > motion->MaxY)
            motion->MaxY = y1;
        y1++;
    }

    //draw current scanline from start position to the bottom
    y1 = y - 1;
    while (y1 >= 0 && PMOTION_XY(motion, x, y1) == 1)
    {
        PMOTION_XY(motion, x, y1) = 2;
        if (y1 < motion->MinY)
            motion->MinY = y1;
        y1--;
    }

    //test for new scanlines to the left and create seeds
    y1 = y;
    while (y1 < motion->size.height && PMOTION_XY(motion, x, y1) == 2)
    {
        if (x > 0 && PMOTION_XY(motion, x - 1, y1) == 1)
            DoNextScanLine(x - 1, y1, motion);
        y1++;
    }
    y1 = y - 1;
    while (y1 >= 0 && PMOTION_XY(motion, x, y1) == 2)
    {
        if (x > 0 && PMOTION_XY(motion, x - 1, y1) == 1)
            DoNextScanLine(x - 1, y1, motion);
        y1--;
    }

    //test for new scanlines to the right
    y1 = y;
    while (y1 < motion->size.height && PMOTION_XY(motion, x, y1) == 2)
    {
        if (x < motion->size.width - 1 && PMOTION_XY(motion, x + 1, y1) == 1)
            DoNextScanLine(x + 1, y1, motion);
        y1++;
    }
    y1 = y - 1;
    while (y1 >= 0 && PMOTION_XY(motion, x, y1) == 2)
    {
        if (x < motion->size.width - 1 && PMOTION_XY(motion, x + 1, y1) == 1)
            DoNextScanLine(x + 1, y1, motion);
        y1--;
    }
}

motionhelper_t GetBoundsFromMotion(motion_t* motion, int sizex, int sizey, int x, int y)
{
    motionhelper_t motionhelper;
    motionhelper.motion = motion->motion;
    motionhelper.MaxX = x;
    motionhelper.MinX = x;
    motionhelper.MaxY = y;
    motionhelper.MinY = y;
    motionhelper.size.width = sizex;
    motionhelper.size.height = sizey;
    DoNextScanLine(x, y, &motionhelper);
    return motionhelper;
}

CDetector::CDetector(imagesize_t Size)
{
    m_sSize = Size;
    m_pLastImage = 0;
    m_flDiffrenceThreshold = 10.0f;
}

CDetector::~CDetector()
{
    delete m_pLastImage;
}

void CDetector::PushImage(CDetectorImage* Image)
{
}

int CDetector::GetTargets(target_t* Targets[MAX_TARGETS])
{
    Targets = new target_t[MAX_TARGETS];

    return 0;
}

void CDetector::SetDiffrenceThreshold(float flAmmount)
{
    m_flDiffrenceThreshold = flAmmount;
}
/*

/// <summary>
/// Uses a FloodFill scanline algorithm to get a targets bounds
/// </summary>
public MotionHelper GetBoundsFromMotion(ref byte[,] motion, int sizex, int sizey, int x, int y)
{
    MotionHelper helper = new MotionHelper(motion, size.X, size.Y, seed.X, seed.Y);
    DoNextScanLine(seed.X, seed.Y, ref helper); // +2 and +1 are for creating an empty  box around the whole shape, (1's never at the end /start of an array)
    helper.Shape = new byte[helper.MaxX - helper.MinX + 4, helper.MaxY - helper.MinY + 4];

    for (int x = helper.MinX; x < helper.MaxX; x++)
        for (int y = helper.MinY; y < helper.MaxY; y++)
            if (helper.Motion[x, y] == 2)
            {
                helper.Shape[x - helper.MinX + 2, y - helper.MinY + 2] = 1;
            }

    return helper;
}

*/
