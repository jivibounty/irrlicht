
#include "Box.h"

namespace irr
{

	Box::Box()
		: m_bResetUpdateExtents(true)
	{
		for (unsigned int planeIndex = 0; planeIndex < NUM_BOX_PLANES; ++planeIndex)
		{
			m_PlaneCoherencyTestIndices[planeIndex] = planeIndex;
		}
	}

	Box::Box(const irr::core::vector3df& center, const irr::core::vector3df& extents)
		: m_bResetUpdateExtents(true)
	{
		for (unsigned int planeIndex = 0; planeIndex < NUM_BOX_PLANES; ++planeIndex)
		{
			m_PlaneCoherencyTestIndices[planeIndex] = planeIndex;
		}
		create(center, extents);
	}

	void Box::getVertices(irr::core::vector3df pVertices[8])
	{
		pVertices[0] = m_Min;
		pVertices[1] = irr::core::vector3df(m_Min.X, m_Min.Y, m_Max.Z);
		pVertices[2] = irr::core::vector3df(m_Max.X, m_Min.Y, m_Max.Z);
		pVertices[3] = irr::core::vector3df(m_Max.X, m_Min.Y, m_Min.Z);
		pVertices[4] = irr::core::vector3df(m_Min.X, m_Max.Y, m_Min.Z);
		pVertices[5] = irr::core::vector3df(m_Min.X, m_Max.Y, m_Max.Z);
		pVertices[6] = m_Max;
		pVertices[7] = irr::core::vector3df(m_Max.X, m_Max.Y, m_Min.Z);
	}

	void Box::create(const irr::core::vector3df& center, const irr::core::vector3df& extents)
	{
		m_Min = center - extents;
		m_Max = center + extents;
		makePlanes();
	}

	void Box::updateExtents(const irr::core::vector3df& vertex)
	{
		if (m_bResetUpdateExtents)
		{
			m_bResetUpdateExtents = false;
			m_Min = vertex;
			m_Max = vertex;
			return;
		}
		if (vertex.X < m_Min.X)
		{
			m_Min.X = vertex.X;
		}
		else if (vertex.X > m_Max.X)
		{
			m_Max.X = vertex.X;
		}

		if (vertex.Y < m_Min.Y)
		{
			m_Min.Y = vertex.Y;
		}
		else if (vertex.Y > m_Max.Y)
		{
			m_Max.Y = vertex.Y;
		}

		if (vertex.Z < m_Min.Z)
		{
			m_Min.Z = vertex.Z;
		}
		else if (vertex.Z > m_Max.Z)
		{
			m_Max.Z = vertex.Z;
		}
	}

	void Box::updateExtentsAndPlanes(const irr::core::vector3df& vertex)
	{
		updateExtents(vertex);
		makePlanes();
	}

	irr::core::vector3df Box::getPosition() const
	{
		return (m_Max + m_Min) * 0.5f;
	}

	void Box::setHalfExtent(float size)
	{
		setHalfExtent(irr::core::vector3df(size, size, size));
	}

	void Box::setHalfExtent(const irr::core::vector3df& size)
	{
		create(getPosition(), size);
	}

	irr::core::vector3df Box::getHalfExtent() const
	{
		return (m_Max - m_Min) * 0.5f;
	}

	const Box& Box::operator=(const Box& box)
	{
		m_Min = box.m_Min;
		m_Max = box.m_Max;

		for (unsigned int planeIndex = 0; planeIndex < NUM_BOX_PLANES; ++planeIndex)
		{
			m_Planes[planeIndex] = box.m_Planes[planeIndex];
		}

		return *this;
	}

	void Box::pad(const irr::core::vector3df& pad)
	{
		m_Min -= pad;
		m_Max += pad;
		makePlanes();
	}

	bool Box::isInBox(const irr::core::vector3df& vertex)
	{
		bool bX_In = vertex.X >= m_Min.X && vertex.X <= m_Max.X;
		bool bY_In = vertex.Y >= m_Min.Y && vertex.Y <= m_Max.Y;
		bool bZ_In = vertex.Z >= m_Min.Z && vertex.Z <= m_Max.Z;
		return bX_In && bY_In && bZ_In;
	}
	bool Box::isInBox(const irr::core::vector3df& vertexA, const irr::core::vector3df& vertexB, const irr::core::vector3df& vertexC)
	{
		for (unsigned int planeIndex = 0; planeIndex < NUM_BOX_PLANES; ++planeIndex)
		{
			unsigned int pos = 0;
			if (m_Planes[planeIndex].signedDistanceTo(vertexA) > 0.0f)
			{
				++pos;
			}
			if (m_Planes[planeIndex].signedDistanceTo(vertexB) > 0.0f)
			{
				++pos;
			}
			if (m_Planes[planeIndex].signedDistanceTo(vertexC) > 0.0f)
			{
				++pos;
			}
			if (pos == 3)
			{
				return false;
			}
		}

		return  true;
	}

	void Box::makePlanes()
	{
		m_Extents = (m_Max - m_Min) * 0.5f;
		m_Planes[0].makePlane(m_Max, irr::core::vector3df(1.0f, 0.0f, 0.0f));
		m_Planes[1].makePlane(m_Max, irr::core::vector3df(0.0f, 1.0f, 0.0f));
		m_Planes[2].makePlane(m_Max, irr::core::vector3df(0.0f, 0.0f, 1.0f));
		m_Planes[3].makePlane(m_Min, irr::core::vector3df(-1.0f, 0.0f, 0.0f));
		m_Planes[4].makePlane(m_Min, irr::core::vector3df(0.0f, -1.0f, 0.0f));
		m_Planes[5].makePlane(m_Min, irr::core::vector3df(0.0f, 0.0f, -1.0f));
	}

	void Box::resetUpdateExtents()
	{
		m_bResetUpdateExtents = true;
	}

	irr::core::vector3df Box::getUpVector(unsigned int planeIndex) const
	{
		switch (planeIndex)
		{
		case 0:
		case 3:
			return irr::core::vector3df(0.0f, 1.0f, 0.0f);
			break;

		case 1:
		case 4:
			return irr::core::vector3df(1.0f, 0.0f, 0.0f);
			break;

		case 2:
		case 5:
			return irr::core::vector3df(0.0f, 1.0f, 0.0f);
			break;
		}

		return irr::core::vector3df(0.0f, 1.0f, 0.0f);
	}

	irr::core::vector3df Box::getPVertex(const irr::core::vector3df& norm) const
	{
		irr::core::vector3df ret(m_Min);

		if (norm.X >= 0.0f)
		{
			ret.X = m_Max.X;
		}

		if (norm.Y >= 0.0f)
		{
			ret.Y = m_Max.Y;
		}

		if (norm.Z >= 0.0f)
		{
			ret.Z = m_Max.Z;
		}
		return ret;
	}

	irr::core::vector3df Box::getNVertex(const irr::core::vector3df& norm) const
	{
		irr::core::vector3df ret(m_Max);

		if (norm.X >= 0.0f)
		{
			ret.X = m_Min.X;
		}

		if (norm.Y >= 0.0f)
		{
			ret.Y = m_Min.Y;
		}

		if (norm.Z >= 0.0f)
		{
			ret.Z = m_Min.Z;
		}
		return ret;
	}

}
