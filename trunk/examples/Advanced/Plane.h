
#pragma once

#include <irrlicht.h>

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

namespace irr
{

	class Plane
	{
	public:
		Plane();
		Plane(const irr::core::vector3df& pointA, const irr::core::vector3df& pointB, const irr::core::vector3df& pointC);
		Plane(const irr::core::vector3df& point, const irr::core::vector3df& norm);
		Plane(float x, float y, float z, float dist);
		Plane(const Plane& plane);

		void makePlane(const irr::core::vector3df& pointA, const irr::core::vector3df& pointB, const irr::core::vector3df& pointC);
		void makePlane(const irr::core::vector3df& point, const irr::core::vector3df& norm);
		void makePlane(float x, float y, float z, float dist);

		irr::core::vector3df split(const irr::core::vector3df& pointA, const irr::core::vector3df& pointB);
		bool isFrontFacingTo(const irr::core::vector3df& direction);
		float signedDistanceTo(const irr::core::vector3df& point);

		const Plane& operator= (const Plane& plane);
		bool operator== (const Plane& plane);
		bool operator!= (const Plane& plane);
		void normalize();

		irr::core::vector3df getNormal();
		float getDistance();
	public:
#pragma warning ( push )
#pragma warning ( disable : 4251 ) 
		irr::core::vector3df m_Normal;
#pragma warning ( pop )
		float m_fDistance;
	};

}
