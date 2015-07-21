
#include "Plane.h"

#define EPSILON 0.00001f

namespace irr
{

	Plane::Plane()
	{
		m_Normal = irr::core::vector3df(1.0f, 0.0f, 0.0f);
		m_fDistance = 0.0f;
	}

	Plane::Plane(const irr::core::vector3df& pointA, const irr::core::vector3df& pointB, const irr::core::vector3df& pointC)
	{
		makePlane(pointA, pointB, pointC);
	}

	Plane::Plane(const irr::core::vector3df& point, const irr::core::vector3df& norm)
	{
		makePlane(point, norm);
	}

	Plane::Plane(float x, float y, float z, float dist)
	{
		makePlane(x, y, z, dist);
	}

	Plane::Plane(const Plane& plane)
	{
		m_Normal = plane.m_Normal;
		m_fDistance = plane.m_fDistance;
	}

	void Plane::makePlane(const irr::core::vector3df& pointA, const irr::core::vector3df& pointB, const irr::core::vector3df& pointC)
	{
		m_Normal = (pointB - pointA).crossProduct(pointC - pointA);
		m_Normal = m_Normal.normalize();
		m_fDistance = -m_Normal.dotProduct(pointA);
	}

	void Plane::makePlane(const irr::core::vector3df& point, const irr::core::vector3df& norm)
	{
		m_Normal = norm;
		m_fDistance = -m_Normal.dotProduct(point);
	}

	void Plane::makePlane(float x, float y, float z, float dist)
	{
		m_Normal = irr::core::vector3df(x, y, z);
		m_fDistance = dist;
		normalize();
	}


	bool Plane::isFrontFacingTo(const irr::core::vector3df& direction)
	{
		float nDot = m_Normal.dotProduct(direction);
		return nDot <= 0.0f;
	}

	float Plane::signedDistanceTo(const irr::core::vector3df& point)
	{
		return m_Normal.dotProduct(point) + m_fDistance;
	}

	irr::core::vector3df Plane::split(const irr::core::vector3df& pointA, const irr::core::vector3df& pointB)
	{
		float fAdot = m_Normal.dotProduct(pointA);
		float fBDot = m_Normal.dotProduct(pointB);
		float fScale = (-m_fDistance - fAdot) / (fBDot - fAdot);
		return pointA + ((pointB - pointA) * fScale);
	}

	bool Plane::operator== (const Plane& plane)
	{
		return (m_Normal == plane.m_Normal) && (fabs(m_fDistance - plane.m_fDistance) <= EPSILON);
	}

	bool Plane::operator!= (const Plane& plane)
	{
		return (m_Normal != plane.m_Normal) || (fabs(m_fDistance - plane.m_fDistance) > EPSILON);
	}

	const Plane& Plane::operator= (const Plane& plane)
	{
		m_Normal = plane.m_Normal;
		m_fDistance = plane.m_fDistance;
		return *this;
	}

	void Plane::normalize()
	{
		float fLen = m_Normal.getLength();
		if (fLen > 0.0f)
		{
			float fReciprocalLen = 1.0f / fLen;
			m_Normal *= fReciprocalLen;
			m_fDistance *= fReciprocalLen;
		}
	}

	irr::core::vector3df Plane::getNormal()
	{
		return m_Normal;
	}

	float Plane::getDistance()
	{
		return m_fDistance;
	}

}
