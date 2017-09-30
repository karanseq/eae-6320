// Include Files
//==============

#include "cSprite.h"

#include "VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/sVector2d.h>

#include <new>

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cSprite::Create(cSprite*& o_sprite, const eae6320::Math::sVector2d& i_origin, const eae6320::Math::sVector2d& i_extents)
{
    auto result = Results::Success;

    cSprite* newSprite = nullptr;

    // Allocate a new sprite
    {
        newSprite = new (std::nothrow) cSprite();
        if (newSprite == nullptr)
        {
            result = Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Couldn't allocate memory for the new sprite!");
            Logging::OutputError("Failed to allocated memory for the new sprite!");
            goto OnExit;
        }
    }

    // Initialize the new sprite's geometry
    if (!(result = newSprite->Initialize(i_origin, i_extents)))
    {
        EAE6320_ASSERTF(false, "Could not initialize the new sprite!");
        goto OnExit;
    }

OnExit:

    if (result)
    {
        EAE6320_ASSERT(newSprite);
        o_sprite = newSprite;
    }
    else
    {
        if (newSprite)
        {
            newSprite->DecrementReferenceCount();
            newSprite = nullptr;
        }
        o_sprite = nullptr;
    }

    return result;
}

eae6320::Graphics::cSprite::~cSprite()
{
    CleanUp();
}

void eae6320::Graphics::cSprite::GetVertexPositions(eae6320::Graphics::VertexFormats::sSprite* o_vertexData, const eae6320::Math::sVector2d& i_origin, const eae6320::Math::sVector2d& i_extents) const
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
