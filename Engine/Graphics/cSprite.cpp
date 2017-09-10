// Include Files
//==============

#include "cSprite.h"

#include "VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Math/sVector2d.h>

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::Graphics::cSprite::cSprite(const eae6320::Math::sVector2d& i_centre, const eae6320::Math::sVector2d& i_extents)
	: m_centre(i_centre), m_extents(i_extents)
{

}

eae6320::Graphics::cSprite::~cSprite()
{
	CleanUp();
}

void eae6320::Graphics::cSprite::GetVertexData(eae6320::Graphics::VertexFormats::sSprite* o_vertexData) const
{
	EAE6320_ASSERT(o_vertexData);

	// bottom-left
	o_vertexData[0].x = m_centre.x - m_extents.x;
	o_vertexData[0].y = m_centre.y - m_extents.y;

	// bottom-right
	o_vertexData[1].x = m_centre.x + m_extents.x;
	o_vertexData[1].y = m_centre.y - m_extents.y;

	// top-right
	o_vertexData[2].x = m_centre.x + m_extents.x;
	o_vertexData[2].y = m_centre.y + m_extents.y;

	// bottom-left
	o_vertexData[3].x = m_centre.x - m_extents.x;
	o_vertexData[3].y = m_centre.y - m_extents.y;

	// top-right
	o_vertexData[4].x = m_centre.x + m_extents.x;
	o_vertexData[4].y = m_centre.y + m_extents.y;

	// top-left
	o_vertexData[5].x = m_centre.x - m_extents.x;
	o_vertexData[5].y = m_centre.y + m_extents.y;
}
