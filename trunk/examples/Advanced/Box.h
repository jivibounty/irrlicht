
#pragma once

#include <irrlicht.h>
#include "Plane.h"

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif


namespace irr
{

#define NUM_BOX_PLANES 6

	class Box
	{
	public:
		//default constructor
		Box();

		//constructor
		Box(const irr::core::vector3df& center, const irr::core::vector3df& extents);

		//gets Box's 8 vertices
		void getVertices(irr::core::vector3df pVertices[8]);

		//initializes box
		void create(const irr::core::vector3df& center, const irr::core::vector3df& extents);

		//updates box extent with vertex
		void updateExtents(const irr::core::vector3df& vertex);

		//updates box extent with vertex and updates box planes
		void updateExtentsAndPlanes(const irr::core::vector3df& vertex);

		//gets position
		irr::core::vector3df getPosition() const;

		//sets half extent with same size for all axis
		void setHalfExtent(float size);

		//sets half extent
		void setHalfExtent(const irr::core::vector3df& size);

		//gets half extent
		irr::core::vector3df getHalfExtent() const;
		const Box& operator=(const Box& box);
		void pad(const irr::core::vector3df& pad);
		bool isInBox(const irr::core::vector3df& vertex);
		bool isInBox(const irr::core::vector3df& vertexA, const irr::core::vector3df& vertexB, const irr::core::vector3df& vertexC);
		void makePlanes();
		void resetUpdateExtents();
		irr::core::vector3df getUpVector(unsigned int planeIndex) const;
		irr::core::vector3df getPVertex(const irr::core::vector3df& norm) const;
		irr::core::vector3df getNVertex(const irr::core::vector3df& norm) const;
	public:
#pragma warning ( push )
#pragma warning ( disable : 4251 ) 
		irr::core::vector3df m_Min;
		irr::core::vector3df m_Max;
		irr::core::vector3df m_Extents;
#pragma warning ( pop )
		Plane m_Planes[NUM_BOX_PLANES];
		unsigned int m_PlaneCoherencyTestIndices[NUM_BOX_PLANES];
		bool m_bResetUpdateExtents;
	};

}
