#ifndef _DET_INTERFACE_LOAD
#error Do not include __FILE__ directly!
#endif
#undef _DET_INTERFACE_LOAD

#ifndef LIB_DET_INTERFACE_H
#define LIB_DET_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>
#include <list>

#define MAX_TARGETS 25 // 25 Seems enough, want more? recompile

#ifdef DETECTOR_INTERNAL


// Helper crap to save time
#define MOTION_XY(_struct_, x, y) _struct_.motion[(x) + (y) * _struct_.size.width]
#define PMOTION_XY(_struct_, x, y) _struct_->motion[(x) + (y) * _struct_->size.width]

#define XY_LOOP(_w_,_h_) for(int y = 0; y < _h_; y++) for(int x = 0; x < _w_; x++)

#define XY_LOOP_START(_x_,_y_,_endx_,_endy_) \
    for(int y = _y_; y < _endy_; y++)\
    for(int x = _x_; x < _endx_; x++)

#define PRINT(_X_) std::cout << _X_ << '\n'

unsigned char DiffrenceBetween( unsigned char a, unsigned char b );

#endif // DETECTOR_INTERNAL

namespace Detector
{

    struct imagesize_t
    {
        int width;
        int height;
    };

    bool imagesize_tEqual( imagesize_t a, imagesize_t b );

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

    struct pixel_t
    {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };

    typedef pixel_t color_t; // Whatever makes sense to you

    class CDetectorBaseClass
    {
    public:
        virtual ~CDetectorBaseClass() {}; // GARH, this is so our real destructor is called!
        void Refrence( void )
        {
            m_iRefrenceCount++;
        };
        void UnRefrence( void )
        {
            m_iRefrenceCount--;
            if( m_iRefrenceCount == 0 )
                delete this;
        };
    protected:
        int m_iRefrenceCount;
    };


    class CDetectorImage : public CDetectorBaseClass // Immutable
    {
    public:
        CDetectorImage( int Width, int Height )
        {
            this->Refrence();
            m_sSize.width = Width;
            m_sSize.height = Height;
            m_psPixels = new pixel_t[Width + Width * Height];
        };
        CDetectorImage( imagesize_t size )
        {
            this->Refrence();
            m_sSize.width = size.width;
            m_sSize.height = size.height;
            m_psPixels = new pixel_t[size.width + size.width * size.height];
        };
        ~CDetectorImage()
        {
            delete[] m_psPixels;
        };
        CDetectorImage* Exclusive()
        {
            if( m_iRefrenceCount == 1 )
                return this;
            imagesize_t selfsize = this->GetSize();
            int size_total = ( selfsize.width + selfsize.width * selfsize.height ) * 3; // 3 bytes per pixel;
            CDetectorImage* ptr = new CDetectorImage( selfsize );
            memcpy( ptr->m_psPixels, this->m_psPixels, size_total );
            this->UnRefrence();
            return ptr;
        };
        pixel_t* Pixel( int x, int y )
        {
            return &m_psPixels[x + y * m_sSize.width];
        };
        imagesize_t GetSize()
        {
            return m_sSize;
        };
    private:
        imagesize_t m_sSize;
        pixel_t* m_psPixels;
    };

    struct target_t
    {
        float x;
        float y;
        float width;
        float height;
        unsigned char motion[];
    };

    class IDetector : public CDetectorBaseClass
    {
    public:
        // Push the next image here, calculates the new target
        virtual bool PushImage( CDetectorImage* Image ) = 0;
        // Returns the number of targets, outputs to argument
        virtual int GetTargets( target_t* Targets[MAX_TARGETS] ) = 0;
    };

    struct velocity_t
    {
        float x;
        float y;
    };

    struct position_t
    {
        float x;
        float y;
    };

    float Distance(position_t &a, position_t &b);
    void MotionBlur(CDetectorImage* Refrence, CDetectorImage* New, float flBlurAmmount, float flMaxChange);
    void BlurMotion(motion_t* Motion);

    struct ssize_t
    {
        float w;
        float h;
    };

    double GetCurrentTime();

    class IDetectorObjectTracker;

    typedef unsigned int targetid;
    class CTrackedObject : public CDetectorBaseClass
    {
        friend class IDetectorObjectTracker;
    public:
        CTrackedObject( targetid ID );
        ~CTrackedObject();
        targetid    ID();
        position_t  Position();
        velocity_t  Velocity();
        ssize_t     Size();
        double      LastSeen();
        float       GetScore(target_t* Target);
        void        Update(position_t& pos, ssize_t& size);
        void        SimulateUpdate(); // If the target was not found
        bool operator ==(CTrackedObject* a);
    private:
        ssize_t     m_sSize;
        targetid    m_tiID;
        velocity_t  m_sVelocity;
        position_t  m_sPosition;
        double      m_dblLastSeen;
    };

    typedef std::list<CTrackedObject*> TrackedObjects;

    class IObjectTracker : public CDetectorBaseClass
    {
    public:
        virtual void PushTargets( target_t* Targets[MAX_TARGETS], int Count ) = 0;
        virtual TrackedObjects* GetTrackedObjects() = 0;
    };

    class CDescriptorValue : public CDetectorBaseClass
    {
    public:
        CDescriptorValue(){ m_iRefrenceCount = 1; }
        ~CDescriptorValue();
        int g_Count;
        float* g_Values;
    };

    class IDescriptor : public CDetectorBaseClass
    {
    public:
        // Returns an array of floats to discribe an object
        virtual CDescriptorValue* GetDescriptor(motion_t* Motion) = 0;
        virtual char* GetName(CDescriptorValue* Descriptor, int Count) = 0;
    };

}
#endif // LIB_DET_INTERFACE_H
