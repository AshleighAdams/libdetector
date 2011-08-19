
#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../header/libdetector.h"
using namespace Detector;

unsigned char DiffrenceBetween( unsigned char a, unsigned char b )
{
	short x = ( short )a - ( short )b;
	if( x < 0 ) return ( unsigned char ) - x;
	return ( unsigned char )x;
}

motion_t* Detector::AbsoluteDiffrence( CDetector* self, CDetectorImage* img1, CDetectorImage* img2 )
{
	motion_t* motion = new motion_t();
	motion->size = img1->GetSize();
	int w, h;
	w = motion->size.width;
	h = motion->size.height;
	motion->motion = new unsigned char[w+h*w];

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
			PMOTION_XY( motion, x, y ) = PIXEL_MOTION;
		else
			PMOTION_XY( motion, x, y ) = PIXEL_NOMOTION;
	}
	return motion;
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
	m_pLastImage = 0;
	m_sDiffrenceThreshold = 10;
	m_iTargets = 0;
	m_flMinTargSize = 0.01f;
	for( int i = 0; i < MAX_TARGETS; i++ )
		m_pTargets[i] = 0;
}

CDetector::~CDetector()
{
	delete m_pLastImage;
}

bool CDetector::PushImage( CDetectorImage* pImage )
{
	if( !imagesize_tEqual( pImage->GetSize(), m_sSize ) )
		return false;
	pImage->Refrence();
	if( !m_pLastImage )
	{
		m_pLastImage = pImage->Exclusive();
		return false;
	}

	// Clear the target cache
	for( int i = 0; i < MAX_TARGETS; i++ )
	{
		if( m_pTargets[i] )
			delete m_pTargets[i];
		m_pTargets[i] = 0;
	}
	motion_t* motion = AbsoluteDiffrence( this, pImage, m_pLastImage );
	int w, h;
	w = motion->size.width;
	h = motion->size.height;
	int count = 0;
	XY_LOOP( w, h )
	{
		if( PMOTION_XY( motion, x, y ) == PIXEL_MOTION )
		{
			motionhelper_t* helper = GetBoundsFromMotion( motion, w, h, x, y );

			target_t* targ = new target_t;
			targ->x = ( float )helper->MinX / ( float )w;
			targ->y = ( float )helper->MinY / ( float )h;
			targ->width = ( float )( helper->MaxX - targ->x ) / ( float )w;
			targ->height = ( float )( helper->MaxY - targ->y ) / ( float )h;

            if( true && targ->height + targ->width < m_flMinTargSize )
            // TODO: Fix this so it only turns on if object recognition is running
            {
                motion_t* movement = new motion_t;
                movement->size.height = helper->MaxY - helper->MinY;
                movement->size.width = helper->MaxX - helper->MinX;
                movement->motion = new unsigned char[x * y * movement->size.width];

                XY_LOOP_START(helper->MinX, helper->MinY,
                              helper->MaxX, helper->MaxY)
                {
                    if(PMOTION_XY(helper, x, y) == PIXEL_SCANNEDMOTION)
                        PMOTION_XY(movement, x, y) = PIXEL_MOTION;
                }

                float discriptor = GetDiscriptor(movement);

                delete movement->motion;
                delete movement;
            }

			delete helper;

			if( targ->height + targ->width < m_flMinTargSize )
			{
				delete targ; // Pfft, this target is too small
				continue;
			}

			m_pTargets[count++] = targ;
			if( count == MAX_TARGETS ) // Max targets have been reached, just escape the loop
				goto EndLoop;
		}
	}

EndLoop:

	m_iTargets = count;
	m_pLastImage->UnRefrence();
	m_pLastImage = pImage->Exclusive();

	delete [] motion->motion;
	delete motion;

	return false;
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

