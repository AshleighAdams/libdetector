
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

