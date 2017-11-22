// Include Files
//==============

#include "../cMeshBuilder.h"

#include <Engine/Graphics/VertexFormats.h>

// Implementation
//===============

void eae6320::Assets::cMeshBuilder::PerformPlatformSpecificFixup(const uint16_t i_vertexCount, eae6320::Graphics::VertexFormats::sMesh* io_vertexData, const uint16_t i_indexCount, uint16_t* io_indexData)
{
    // D3D uses top-to-bottom UVs so reverse the Vs appropriately
    for (uint16_t i = 0; i < i_vertexCount; ++i)
    {
        io_vertexData[i].v = 1.0f - io_vertexData[i].v;
    }

    // D3D uses clockwise winding so swap the indices accordingly
    static constexpr uint8_t s_indicesPerTriangle = 3;
    const uint16_t numTriangles = i_indexCount / s_indicesPerTriangle;
    for (uint16_t i = 0; i < numTriangles; ++i)
    {
        std::swap(io_indexData[i * s_indicesPerTriangle + 1], io_indexData[i * s_indicesPerTriangle + 2]);
    }
}
