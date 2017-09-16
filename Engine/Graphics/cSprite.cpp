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

eae6320::Graphics::cSprite::~cSprite()
{
	CleanUp();
}

void eae6320::Graphics::cSprite::GetVertexData(eae6320::Graphics::VertexFormats::sSprite* o_vertexData, const eae6320::Math::sVector2d& i_origin, const eae6320::Math::sVector2d& i_extents) const
{
	EAE6320_ASSERT(o_vertexData);

	// bottom-right
	o_vertexData[0].x = i_origin.x + i_extents.x;
	o_vertexData[0].y = i_origin.y - i_extents.y;

	// top-right
	o_vertexData[1].x = i_origin.x + i_extents.x;
	o_vertexData[1].y = i_origin.y + i_extents.y;

	// bottom-left
	o_vertexData[2].x = i_origin.x - i_extents.x;
	o_vertexData[2].y = i_origin.y - i_extents.y;

	// top-left
	o_vertexData[3].x = i_origin.x - i_extents.x;
	o_vertexData[3].y = i_origin.y + i_extents.y;
}
