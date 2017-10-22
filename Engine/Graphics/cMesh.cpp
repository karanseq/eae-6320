// Include Files
//==============

#include "cMesh.h"

#include "VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Graphics/sColor.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/sVector.h>

#include <new>

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cMesh::Create(cMesh*& o_mesh, const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const uint16_t* i_indices, const eae6320::Graphics::sColor* i_colors)
{
    auto result = Results::Success;

    // Validate inputs
    {
        if (i_vertexCount % s_verticesPerTriangle)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "The vertex count for a mesh must be a multiple of 3!");
            Logging::OutputError("The vertex count for a mesh must be a multiple of 3!");
            goto OnExit;
        }
        else if (i_vertices == nullptr)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "Invalid vertex data!");
            Logging::OutputError("Invalid vertex data!");
            goto OnExit;
        }
        else if (i_indices == nullptr)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "Invalid index data!");
            Logging::OutputError("Invalid index data!");
            goto OnExit;
        }
        else if (i_colors == nullptr)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "Invalid color data!");
            Logging::OutputError("Invalid color data!");
            goto OnExit;
        }
    }

    cMesh* newMesh = nullptr;

    // Allocate a new mesh
    {
        newMesh = new (std::nothrow) cMesh();
        if (newMesh == nullptr)
        {
            result = Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Couldn't allocate memory for the new mesh!");
            Logging::OutputError("Failed to allocated memory for the new mesh!");
            goto OnExit;
        }
    }

    // Initialize the new mesh's geometry
    if (!(result = newMesh->Initialize(i_vertexCount, i_vertices, i_indices, i_colors)))
    {
        EAE6320_ASSERTF(false, "Could not initialize the new mesh!");
        goto OnExit;
    }

OnExit:

    if (result)
    {
        EAE6320_ASSERT(newMesh);
        o_mesh = newMesh;
    }
    else
    {
        if (newMesh)
        {
            newMesh->DecrementReferenceCount();
            newMesh = nullptr;
        }
        o_mesh = nullptr;
    }

    return result;
}

eae6320::Graphics::cMesh::~cMesh()
{
    CleanUp();
}

// Implementation
//===============

void eae6320::Graphics::cMesh::GetVertexBufferData(VertexFormats::sMesh* o_vertexData, const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const eae6320::Graphics::sColor* i_colors) const
{
    EAE6320_ASSERT(o_vertexData != nullptr && i_vertexCount % s_verticesPerTriangle == 0);

    for (uint16_t i = 0; i < i_vertexCount; ++i)
    {
        o_vertexData[i].x = i_vertices[i].x;
        o_vertexData[i].y = i_vertices[i].y;
        o_vertexData[i].r = static_cast<uint8_t>(i_colors[i].r * 255.0f);
        o_vertexData[i].g = static_cast<uint8_t>(i_colors[i].g * 255.0f);
        o_vertexData[i].b = static_cast<uint8_t>(i_colors[i].b * 255.0f);
        o_vertexData[i].a = static_cast<uint8_t>(i_colors[i].a * 255.0f);
    }
}

void eae6320::Graphics::cMesh::GetIndexBufferData(uint16_t* o_indexData, const uint16_t i_vertexCount, const uint16_t* i_indices) const
{
    EAE6320_ASSERT(o_indexData != nullptr && i_vertexCount % s_verticesPerTriangle == 0);

    for (uint16_t i = 0; i < i_vertexCount; ++i)
    {
        o_indexData[i] = i_indices[i];
    }
}
