
#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#include "../header/libdetector.h"
using namespace Detector;

// Helper crap to save time
#define MOTION_XY(_struct_, x, y) _struct_.motion[(x) + (y) * _struct_.size.width]
#define PMOTION_XY(_struct_, x, y) _struct_->motion[(x) + (y) * _struct_->size.width]

#define XY_LOOP(_w_,_h_) for(int y = 0; y < _h_; y++) for(int x = 0; x < _w_; x++)
#define PRINT(_X_) std::cout << _X_ << '\n'

bool Detector::imagesize_tEqual( imagesize_t a, imagesize_t b )
{
	return a.width == b.width && a.height == b.height;
}

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
	motionhelper->size.width = sizex;
	motionhelper->size.height = sizey;
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


/// Tracked object stuff
CTrackedObject::CTrackedObject( targetid ID )
{
	m_tiID = ID;
}

CTrackedObject::~CTrackedObject()
{} // Unused for now

double CTrackedObject::LastSeen()
{
	return m_dblLastSeen;
}

void CTrackedObject::Update(position_t& pos, ssize_t& size)
{
    float velx = pos.x - m_sPosition.x;
    float vely = pos.y - m_sPosition.y;
    m_sPosition = pos;
    m_sVelocity.x = velx;
    m_sVelocity.y = vely;

    m_sSize.w = size.w;
    m_sSize.h = size.h;

    m_dblLastSeen = GetCurrentTime();
}

targetid CTrackedObject::ID()
{
    return m_tiID;
}

void CTrackedObject::SimulateUpdate()
{
    m_sPosition.x += m_sVelocity.x;
    m_sPosition.y += m_sVelocity.y;
}

bool CTrackedObject::operator==(CTrackedObject* a)
{
    return a->m_tiID == this->m_tiID;
}

float CTrackedObject::GetScore(target_t* Target)
{
    position_t targpos;
    targpos.x = Target->x;
    targpos.y = Target->y;


    float score_pos = 1.0f - std::min(1.0f, Distance(m_sPosition, targpos) / 0.3f); // score algo with max being 1 for all of them

    float size_x = 1.0f - std::min(1.0f, std::abs(m_sSize.w - Target->width) / 0.1f); // About 10% of the image error
    float size_y = 1.0f - std::min(1.0f, std::abs(m_sSize.h - Target->height) / 0.1f);

    float score_size = (size_x + size_y) / 2.0f;
    //float score_vel = (vel_x + vel_y) / 2f;

    return (((score_pos + score_size) / 2.0f));
}

// End tracked object stuff

// Object tracker stuff

NewTargetFn         NewTargEvent = NULL;
UpdateTargetFn      UpdateEvent = NULL;
LostTargetFn        LostTargEvent = NULL;


// Stuff that's usefull to the tracker
float Q_sqrt( float number ) // Thanks whoever made this (this implentation is from Quake III Arena)
{
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    return number * y;
}

float Detector::Distance(position_t& a, position_t& b)
{
    float x = std::abs(a.x - b.x);
    float y = std::abs(a.y - b.y);

    return Q_sqrt(x*x+y*y);
}

CObjectTracker::CObjectTracker()
{
    m_flLastSeenLifeTime = 0.5f;
    m_flNewTargetThreshold = 0.7f;
}

CObjectTracker::~CObjectTracker()
{
}

void CObjectTracker::PushTargets(target_t* Targets[MAX_TARGETS], int Count)
{
    if(Count > MAX_TARGETS -1) return; // Just an assertion
    for(int i = 0; i < Count; i++)
    {
        target_t* Target = Targets[i];
        float bestscore = m_flNewTargetThreshold;
        CTrackedObject* best = NULL;

        for(CTrackedObject* Obj : m_TrackedObjects) // Yay, 0x standard!
        {
            float score = Obj->GetScore(Target);
            if(score > bestscore)
            {
                bestscore = score;
                best = Obj;
            }
        }

        position_t pos;
        pos.x = Target->x;
        pos.y = Target->y;

        ssize_t size;
        size.w = Target->width;
        size.h = Target->height;

        if(bestscore > m_flNewTargetThreshold) // If the score is below the threshold then create a new target to match
        {
            best->Update(pos, size);

            if(UpdateEvent)
                UpdateEvent(best, false);
        }
        else
        {
            CTrackedObject* newobj = new CTrackedObject(m_CurrentID++);
            newobj->Update(pos, size);

            m_TrackedObjects.push_back(newobj);

            if(NewTargEvent)
                NewTargEvent(newobj);
        }
    }

    CTrackedObject* RemoveNextIteration = NULL;

    for(CTrackedObject* Obj : m_TrackedObjects)
    {
        if(RemoveNextIteration)
        {
            m_TrackedObjects.remove(RemoveNextIteration);
            RemoveNextIteration->UnRefrence();

            RemoveNextIteration = NULL;
        }

        double lastseen = GetCurrentTime() - Obj->LastSeen();
        if(lastseen > 1.0) // TODO: Make a var to control this
        {
            if(LostTargEvent)
                LostTargEvent(Obj);

            RemoveNextIteration = Obj; // Mark this to be removed next iterate (doing it now will cause a segfault)
            continue;
        }
        else if (lastseen > 0.05) // Simulate stuff if we havn't seen for 50ms
        {
            Obj->SimulateUpdate();
            if(UpdateEvent)
                UpdateEvent(Obj, true);
        }
    }

    if(RemoveNextIteration)
    {
        m_TrackedObjects.remove(RemoveNextIteration);
        RemoveNextIteration->UnRefrence();

        RemoveNextIteration = NULL;
    }
}

TrackedObjects* CObjectTracker::GetTrackedObjects()
{
    return &m_TrackedObjects;
}

void CObjectTracker::SetLastSeenLifeTime(float flAmmount)
{
    m_flLastSeenLifeTime = flAmmount;
}

// Events arn't done yet, only one event currently
void CObjectTracker::SetEvent(EventType type, void* function)
{
    switch(type)
    {
        case EVENT_NEWTARG:
        {
            NewTargEvent = (NewTargetFn)(unsigned long)function;
            break;
        }
        case EVENT_UPDATE:
        {
            UpdateEvent = (UpdateTargetFn)(unsigned long)function;
            break;
        }
        case EVENT_LOST:
        {
            LostTargEvent = (LostTargetFn)(unsigned long)function;
            break;
        }
    }
}
