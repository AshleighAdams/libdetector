#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../include/libdetector.h"
using namespace Detector;

unsigned char DiffrenceBetween( unsigned char a, unsigned char b )
{
	short x = ( short )a - ( short )b;
	if( x < 0 ) return ( unsigned char ) - x;
	return ( unsigned char )x;
}

void Detector::AbsoluteDiffrence( CDetector* self, CDetectorImage* img1, CDetectorImage* img2, motion_t* Target )
{
    int w, h;
	w = Target->size.width;
	h = Target->size.height;

	pixel_t* pix1;
	pixel_t* pix2;

	XY_LOOP( w, h )
	{
		pix1 = img1->Pixel( x, y );
		pix2 = img2->Pixel( x, y );

		unsigned char diff_r = DiffrenceBetween( pix1->r, pix2->r );
		unsigned char diff_g = DiffrenceBetween( pix1->g, pix2->g );
		unsigned char diff_b = DiffrenceBetween( pix1->b, pix2->b );

		short totaldiff = diff_r + diff_g + diff_b;

		if( totaldiff > self->m_sDiffrenceThreshold )
			PMOTION_XY( Target, x, y ) = PIXEL_MOTION;
		else
			PMOTION_XY( Target, x, y ) = PIXEL_NOMOTION;
	}
}

void DoNextScanLine( int x, int y, motionhelper_t* motion )
{
	if( x >= motion->size.width || y >= motion->size.height )
		return;
	if( PMOTION_XY( motion, x, y ) == PIXEL_NOMOTION )
		return;

	if ( x > motion->MaxX )
		motion->MaxX = x;
	if ( x < motion->MinX )
		motion->MinX = x;
	if ( y > motion->MaxY )
		motion->MaxY = y;
	if ( y < motion->MinY )
		motion->MinY = y;

	int y1 = y;

	//draw current scanline from start position to the top
	while ( y1 < motion->size.height && PMOTION_XY( motion, x, y1 ) == PIXEL_MOTION )
	{
		PMOTION_XY( motion, x, y1 ) = PIXEL_SCANNEDMOTION;
		if ( y1 > motion->MaxY )
			motion->MaxY = y1;
		y1++;
	}

	//draw current scanline from start position to the bottom
	y1 = y - 1;
	while ( y1 >= 0 && PMOTION_XY( motion, x, y1 ) == PIXEL_MOTION )
	{
		PMOTION_XY( motion, x, y1 ) = PIXEL_SCANNEDMOTION;
		if ( y1 < motion->MinY )
			motion->MinY = y1;
		y1--;
	}

	//test for new scanlines to the left and create seeds
	y1 = y;
	while ( y1 < motion->size.height && PMOTION_XY( motion, x, y1 ) == PIXEL_SCANNEDMOTION )
	{
		if ( x > 0 && PMOTION_XY( motion, x - 1, y1 ) == PIXEL_MOTION )
			DoNextScanLine( x - 1, y1, motion );
		y1++;
	}
	y1 = y - 1;
	while ( y1 >= 0 && PMOTION_XY( motion, x, y1 ) == PIXEL_SCANNEDMOTION )
	{
		if ( x > 0 && PMOTION_XY( motion, x - 1, y1 ) == PIXEL_MOTION )
			DoNextScanLine( x - 1, y1, motion );
		y1--;
	}

	//test for new scanlines to the right
	y1 = y;
	while ( y1 < motion->size.height && PMOTION_XY( motion, x, y1 ) == PIXEL_SCANNEDMOTION )
	{
		if ( x < motion->size.width - 1 && PMOTION_XY( motion, x + 1, y1 ) == PIXEL_MOTION )
			DoNextScanLine( x + 1, y1, motion );
		y1++;
	}
	y1 = y - 1;
	while ( y1 >= 0 && PMOTION_XY( motion, x, y1 ) == PIXEL_SCANNEDMOTION )
	{
		if ( x < motion->size.width - 1 && PMOTION_XY( motion, x + 1, y1 ) == PIXEL_MOTION )
			DoNextScanLine( x + 1, y1, motion );
		y1--;
	}
}

motionhelper_t* GetBoundsFromMotion( motion_t* motion, int sizex, int sizey, int x, int y )
{
	motionhelper_t* motionhelper = new motionhelper_t;
	motionhelper->motion = motion->motion;
	motionhelper->MaxX = x;
	motionhelper->MinX = x;
	motionhelper->MaxY = y;
	motionhelper->MinY = y;

	// Size of the motion
	motionhelper->size.width = sizex;
	motionhelper->size.height = sizey;

	// Start it.
	DoNextScanLine( x, y, motionhelper );

	return motionhelper;
}

CDetector::CDetector( imagesize_t Size )
{
	m_sSize = Size;
	m_pRefrenceImage = NULL;
	m_pMotionImage = NULL;
	m_sDiffrenceThreshold = 50;

	m_pMotionImage = new motion_t();
	m_pMotionImage->size = Size;
	m_pMotionImage->motion = new unsigned char[Size.width + Size.height * Size.width];

	m_iTargets = 0;
	m_flMinTargSize = .05f;
	for( int i = 0; i < MAX_TARGETS; i++ )
		m_pTargets[i] = NULL;
    m_pDescriptor = NULL;
    m_flBlurAmmount = .1f;
    m_flBlurMaxChange = 1.f;
    m_BlurUpdateRate = 10; // So we can slow the update rate of the background image
    m_BlurUpdateRateFrame = 0;
    m_flTotalMotion = 0.f;

}

CDetector::~CDetector()
{
	m_pRefrenceImage->UnRefrence();
	m_pDescriptor->UnRefrence();
    m_pIgnoreMotionImage->UnRefrence();

	delete [] m_pMotionImage->motion;
    delete m_pMotionImage;

    // Make sure all the targets are deleted
	for( int i = 0; i < MAX_TARGETS; i++ )
	{
		if( m_pTargets[i] )
			delete m_pTargets[i];
		m_pTargets[i] = 0;
	}
}

void CDetector::SetDescriptor(IDescriptor* Descriptor)
{
    if(m_pDescriptor)
        m_pDescriptor->UnRefrence();

    m_pDescriptor = Descriptor;

    if(m_pDescriptor) // Déjà vu (almost)
        m_pDescriptor->Refrence();
}

bool CDetector::PushImage( CDetectorImage* pImage )
{
	if( !imagesize_tEqual( pImage->GetSize(), m_sSize ) )
		return false;
    m_iFalsePos = 0;
	pImage->Refrence();
	if( !m_pRefrenceImage )
	{
		m_pRefrenceImage = pImage->Exclusive();
		return false;
	}

	// Clear the target cache (not reallocate memory, that's slow)
	for( int i = 0; i < MAX_TARGETS; i++ )
	{
		if( m_pTargets[i] )
			delete m_pTargets[i];
		m_pTargets[i] = 0;
	}

	AbsoluteDiffrence( this, pImage, m_pRefrenceImage, m_pMotionImage );
	BlurMotion(m_pMotionImage);

	int w, h;
	w = m_pMotionImage->size.width;
	h = m_pMotionImage->size.height;
	int count = 0;

	XY_LOOP( w, h )
	{
		if( PMOTION_XY( m_pMotionImage, x, y ) == PIXEL_MOTION )
		{
			motionhelper_t* helper = GetBoundsFromMotion( m_pMotionImage, w, h, x, y );

			target_t* targ = new target_t;
			targ->x = ( float )helper->MinX / ( float )w;
			targ->y = ( float )helper->MinY / ( float )h;
			targ->width = ( float )( helper->MaxX - helper->MinX ) / ( float )w;
			targ->height = ( float )( helper->MaxY - helper->MinY ) / ( float )h;

            if( m_pDescriptor && (targ->height + targ->width) > m_flMinTargSize )
            {
                motion_t* movement = new motion_t;
                movement->size.height = helper->MaxY - helper->MinY;
                movement->size.width = helper->MaxX - helper->MinX;
                movement->motion = new unsigned char[movement->size.width + movement->size.height * movement->size.width];

                int minx = helper->MinX;
                int miny = helper->MinY;

                for(int ly = helper->MinY; ly < helper->MaxY; ly++)
                    for(int lx = helper->MinX; lx < helper->MaxX; lx++)
                    {
                        if(PMOTION_XY(helper, lx, ly) == PIXEL_SCANNEDMOTION)
                            PMOTION_XY(movement, lx - minx, ly - miny) = PIXEL_MOTION;
                    }

                // TODO: Discriptor Stuff
                m_pDescriptor->GetDescriptor(movement);

                delete [] movement->motion;
                delete movement;
            }

			delete helper;

			if( targ->height + targ->width < m_flMinTargSize )
			{
				delete targ; // Pfft, this target is too small
				m_iFalsePos++;
				continue;
			}

			m_pTargets[count++] = targ;
			if( count == MAX_TARGETS ) // Max targets have been reached, just escape the loop
				goto EndLoop;   // This is here because we are nested in 2 loops
		}
	}

EndLoop:

	m_iTargets = count;

    int MaxSetPixels = m_sSize.width + m_sSize.height * m_sSize.width;
    int SetPixels = 0;

    XY_LOOP(m_sSize.width, m_sSize.height)
        if(PMOTION_XY(m_pMotionImage, x,y) > 0) SetPixels++;

    m_flTotalMotion = (float)SetPixels / (float)MaxSetPixels;

    if(m_bCleverBackground) // Not implented yet
    {
        if(m_iFalsePos > 5);
        {
            // this will be a per pixel update, no blur, all manual
            MotionBlur(m_pRefrenceImage, pImage, m_flBlurAmmount, m_flBlurMaxChange);
        }
    }else
    {
        m_BlurUpdateRateFrame++;
        if(m_BlurUpdateRateFrame == m_BlurUpdateRate)
        {
            MotionBlur(m_pRefrenceImage, pImage, m_flBlurAmmount, m_flBlurMaxChange);
            m_BlurUpdateRateFrame = 0;
        }else if(m_flTotalMotion > 0.5f)
        {
            MotionBlur(m_pRefrenceImage, pImage, m_flBlurAmmount, 999.f);
        }
    }

	//m_pRefrenceImage->UnRefrence();
	//m_pRefrenceImage = pImage->Exclusive();
	pImage->UnRefrence();

	return true;
}

void CDetector::SetMotionBlur(float flAmmount)
{
    m_flBlurAmmount = flAmmount;
}

int CDetector::GetTargets( target_t* Targets[MAX_TARGETS] )
{
	if( !m_pTargets )
		return 0;

	for( int i = 0; i < MAX_TARGETS; i++ )
		Targets[i] = m_pTargets[i];
	return m_iTargets;
}

int CDetector::GetNumberOfTargets()
{
	return m_iTargets;
}

void CDetector::SetDiffrenceThreshold( short sAmmount )
{
	m_sDiffrenceThreshold = sAmmount;
}

void CDetector::SetMinTargSize( float flAmmount )
{
	m_flMinTargSize = flAmmount;
}

unsigned int CDetector::GetFalsePosCount()
{
    return m_iFalsePos;
}

float CDetector::GetTotalMotion()
{
    return m_flTotalMotion;
}

motion_t* CDetector::GetMotionImage()
{
    return m_pMotionImage;
}

void CDetector::SetIgnoreImage(CDetectorImage* pImage)
{
    if(m_pIgnoreMotionImage)
        m_pIgnoreMotionImage->UnRefrence();

    m_pIgnoreMotionImage = pImage; // TODO: Debating whether to use Exclusive (people can change after it's been set, eg, draw on it

    if(m_pIgnoreMotionImage)
        m_pIgnoreMotionImage->Refrence();
}















