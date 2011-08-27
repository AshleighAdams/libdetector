#include "string.h"
#include <iostream>
#include <exception>
#include <cmath>

#define DETECTOR_INTERNAL
#include "../include/libdetector.h"
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

velocity_t CTrackedObject::Velocity()
{
	return m_sVelocity;
}

Detector::ssize_t CTrackedObject::Size()
{
	return m_sSize;
}

position_t CTrackedObject::Position()
{
	return m_sPosition;
}

position_t CTrackedObject::CenterPosition()
{
	return m_sCenterPosition;
}

void CTrackedObject::Update(position_t& pos, ssize_t& size)
{
	float cx = m_sCenterPosition.x, cy = m_sCenterPosition.y;

	m_sPosition.x = pos.x;
	m_sPosition.y = pos.y;

	m_sSize.w = size.w;
	m_sSize.h = size.h;

	m_sCenterPosition.x = pos.x + size.w * 0.5f;
	m_sCenterPosition.y = pos.y + size.h * 0.5f;

	float velx = m_sCenterPosition.x - cx;
	float vely = m_sCenterPosition.y - cy;

	m_sVelocity.x = velx;
	m_sVelocity.y = vely;

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
