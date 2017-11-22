// Include Files
//==============

#include "cMesh.h"

#include "VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Graphics/sColor.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/sVector.h>
#include <Engine/Math/sVector2d.h>
#include <Engine/Platform/Platform.h>

#include <new>

// Static Data Initialization
//===========================

eae6320::Assets::cManager<eae6320::Graphics::cMesh> eae6320::Graphics::cMesh::s_manager;

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cMesh::Load(const char* const i_path, cMesh*& o_mesh)
{
    auto result = Results::Success;

    Platform::sDataFromFile dataFromFile;

    // Load the binary data
    {
        std::string errorMessage;
        if (!(result = Platform::LoadBinaryFile(i_path, dataFromFile, &errorMessage)))
        {
            EAE6320_ASSERTF(false, errorMessage.c_str());
            Logging::OutputError("Failed to load mesh data from file %s: %s", i_path, errorMessage.c_str());
            goto OnExit;
        }
    }

    // Extract data from the file
    auto currentOffset = reinterpret_cast<uintptr_t>(dataFromFile.data);
    const auto finalOffset = currentOffset + dataFromFile.size;
    EAE6320_ASSERT(finalOffset > currentOffset);

    const uint16_t vertexCount = *(reinterpret_cast<uint16_t*>(currentOffset));
    currentOffset += sizeof(uint16_t);

    VertexFormats::sMesh* vertexData = reinterpret_cast<VertexFormats::sMesh*>(currentOffset);
    currentOffset += vertexCount * sizeof(VertexFormats::sMesh);

    uint16_t indexCount = *(reinterpret_cast<uint16_t*>(currentOffset));
    currentOffset += sizeof(uint16_t);

    uint16_t* indices = reinterpret_cast<uint16_t*>(currentOffset);
    currentOffset += indexCount * sizeof(uint16_t);

    // Make sure we reached the end of the file
    EAE6320_ASSERT(currentOffset == finalOffset);

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
    if (!(result = newMesh->Initialize(vertexCount, vertexData, indexCount, indices)))
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

    dataFromFile.Free();

    return result;
}

eae6320::Graphics::cMesh::~cMesh()
{
    CleanUp();
}
