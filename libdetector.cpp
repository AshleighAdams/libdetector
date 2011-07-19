#include "string.h"
#include "libdetector.h"

// Helper crap to save time
#define MOTION_XY(_struct_, x, y) _struct_.motion[(x) + (y) * _struct_.size.width]
#define PMOTION_XY(_struct_, x, y) _struct_->motion[(x) + (y) * _struct_->size.width]

#define XY_LOOP(_w_,_h_) for(int y = 0; x < _h_; y++) for(int x = 0; x < _w_; x++)

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

unsigned char DiffrenceBetween(unsigned char a, unsigned char b)
{
    short x = (short)a - (short)b;
    if(x<0) return (unsigned char)-x;
    return (unsigned char)x;
}

void AbsoluteDiffrence(CDetector* self, CDetectorImage* img1, CDetectorImage* img2, motion_t* motion)
{
    motion = new motion_t;
    motion->size = img1->GetSize();
    int w,h;
    w = motion->size.width;
    h = motion->size.height;
    motion->motion = new unsigned char[w*h];

    pixel_t* pix1;
    pixel_t* pix2;

    XY_LOOP(w,h)
    {
        img1->Pixel(x, y, pix1);
        img2->Pixel(x, y, pix2);

        unsigned char diff_r = DiffrenceBetween(pix1->r, pix2->r);
        unsigned char diff_g = DiffrenceBetween(pix1->g, pix2->g);
        unsigned char diff_b = DiffrenceBetween(pix1->b, pix2->b);

        short totaldiff = diff_r + diff_g + diff_b;

        if(totaldiff > self->m_sDiffrenceThreshold)
            PMOTION_XY(motion, x, y) = 1;
        else
            PMOTION_XY(motion, x, y) = 0;
    }
}

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

void CDetector::PushImage(CDetectorImage* pImage)
{
    if(!m_pLastImage)
        m_pLastImage = pImage;

    motion_t* motion;
    AbsoluteDiffrence(pImage, m_pLastImage, motion);

    delete motion;
}

int CDetector::GetTargets(target_t* Targets[MAX_TARGETS])
{
    *Targets = new target_t[MAX_TARGETS];
    memcpy(Targets, m_pTargets, sizeof(m_pTargets));
    return 0;
}

void CDetector::SetDiffrenceThreshold(short sAmmount)
{
    m_sDiffrenceThreshold = sAmmount;
}
