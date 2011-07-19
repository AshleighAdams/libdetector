#ifndef LIB_DET_INTERFACE_H
#define LIB_DET_INTERFACE_H
#define MAX_TARGETS 25 // 25 Seems enough, want more? recompile

struct pixel_t
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct imagesize_t
{
    int width;
    int height;
};

class CDetectorImage
{
public:
    CDetectorImage(int Width, int Height)
    {
        m_sSize.width = Width;
        m_sSize.height = Height;
        m_psPixels = new pixel_t[Width * Height];
    };
    ~CDetectorImage()
    {
        delete[] m_psPixels;
    };
    void Pixel(int x, int y, pixel_t *Pixel)
    {
        Pixel = &m_psPixels[x + y * m_sSize.width];
    };
private:
    imagesize_t m_sSize;
    pixel_t* m_psPixels;
};

struct target_t
{
    int x;
    int y;
    int width;
    int height;
};

class IDetector
{
public:
    // Push the next image here, calculates the new target
    virtual void PushImage(CDetectorImage *Image) = 0;
    // Returns the number of targets, outputs to argument
    virtual int GetTargets(target_t* Targets[MAX_TARGETS]) = 0;
};


#endif // LIB_DET_INTERFACE_H
