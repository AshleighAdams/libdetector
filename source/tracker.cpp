
#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../header/libdetector.h"
using namespace Detector;


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
    m_flLastSeenLifeTime        = 0.5f;
    m_flNewTargetThreshold      = 0.7f;
    m_flNewTargetTimeThreshold  = 0.25f; // We have to be waiting this ammount of time before we can create a new target
    m_pNewTargEvent             = NULL;
    m_pUpdateEvent              = NULL;
    m_pLostTargEvent            = NULL;
}

CObjectTracker::~CObjectTracker()
{
}

void CObjectTracker::PushTargets(target_t* Targets[MAX_TARGETS], int Count)
{
    if(Count > MAX_TARGETS -1) return; // Just an assertion
    bool NewTargets = false;
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

            if(m_pUpdateEvent)
                m_pUpdateEvent(best, false);
        }
        else
        {
            if(m_flNewTargetTime != 0.f && GetCurrentTime() - m_flNewTargetTime > m_flNewTargetTimeThreshold)
            {
                CTrackedObject* newobj = new CTrackedObject(m_CurrentID++);
                newobj->Update(pos, size);

                m_TrackedObjects.push_back(newobj);

                if(m_pNewTargEvent)
                    m_pNewTargEvent(newobj);

                m_flNewTargetTime = GetCurrentTime();
            }
            else
                NewTargets = true;

        }
    }

    // This is so a target must be wanted to be created for m_flNewTargetTimeThreshold seconds
    if(NewTargets)
    {
        if(m_flNewTargetTime == 0.f)
            m_flNewTargetTime = GetCurrentTime();
    }
    else
        m_flNewTargetTime = 0.f;

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
            if(m_pLostTargEvent)
                m_pLostTargEvent(Obj);

            RemoveNextIteration = Obj; // Mark this to be removed next iterate (doing it now will cause a segfault)
            continue;
        }
        else if (lastseen > 0.05) // Simulate stuff if we havn't seen for 50ms
        {
            Obj->SimulateUpdate();
            if(m_pUpdateEvent)
                m_pUpdateEvent(Obj, true);
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
            m_pNewTargEvent = (NewTargetFn)(unsigned long)function;
            break;
        }
        case EVENT_UPDATE:
        {
            m_pUpdateEvent = (UpdateTargetFn)(unsigned long)function;
            break;
        }
        case EVENT_LOST:
        {
            m_pLostTargEvent = (LostTargetFn)(unsigned long)function;
            break;
        }
    }
}

