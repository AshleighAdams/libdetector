#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../header/libdetector.h"
using namespace Detector;


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
