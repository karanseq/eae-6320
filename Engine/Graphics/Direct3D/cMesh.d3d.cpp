// Include Files
//==============

#include "../cMesh.h"

#include "Includes.h"
#include "../sContext.h"
#include "../VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>

// Interface
//==========

// Render
//-------

void eae6320::Graphics::cMesh::Draw() const
{
    auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
    EAE6320_ASSERT(direct3dImmediateContext);

    // Bind a specific vertex buffer to the device as a data source
    {
        EAE6320_ASSERT(m_vertexBuffer);
        constexpr unsigned int startingSlot = 0;
        constexpr unsigned int vertexBufferCount = 1;
        // The "stride" defines how large a single vertex is in the stream of data
        constexpr unsigned int bufferStride = sizeof(VertexFormats::sMesh);
        // It's possible to start streaming data in the middle of a vertex buffer
        constexpr unsigned int bufferOffset = 0;
        direct3dImmediateContext->IASetVertexBuffers(startingSlot, vertexBufferCount, &m_vertexBuffer, &bufferStride, &bufferOffset);
    }

    // Specify what kind of data the vertex buffer holds
    {
        // Set the layout (which defines how to interpret a single vertex)
        {
            EAE6320_ASSERT(m_vertexInputLayout);
            direct3dImmediateContext->IASetInputLayout(m_vertexInputLayout);
        }

        // Set the topology 
        direct3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // Bind an input buffer to the device as a data source
    {
        EAE6320_ASSERT(m_indexBuffer);
        // The indices start at the beginning of the buffer
        constexpr unsigned int offset = 0;
        direct3dImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, offset);
    }

    // Render triangles from the currently-bound vertex buffer
    {
        // It's possible to start rendering primitives in the middle of the stream
        constexpr unsigned int indexOfFirstIndexToUse = 0;
        constexpr unsigned int offsetToAddToEachIndex = 0;
        direct3dImmediateContext->DrawIndexed(static_cast<unsigned int>(m_indexCount), indexOfFirstIndexToUse, offsetToAddToEachIndex);
    }
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cMesh::Initialize(const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const eae6320::Math::sVector2d* i_uvs, const uint16_t i_indexCount, const uint16_t* i_indices)
{
    auto result = eae6320::Results::Success;

    // Save the vertex count as the index count
    m_indexCount = i_indexCount;

    auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
    EAE6320_ASSERT(direct3dDevice);

    // Initialize vertex format
    {
        // Load the compiled binary vertex shader for the input layout
        eae6320::Platform::sDataFromFile vertexShaderDataFromFile;
        std::string errorMessage;
        if (result = eae6320::Platform::LoadBinaryFile("data/Shaders/Vertex/vertexInputLayout_mesh.shd", vertexShaderDataFromFile, &errorMessage))
        {
            // Create the vertex layout

            // These elements must match the VertexFormats::sMesh layout struct exactly.
            // They instruct Direct3D how to match the binary data in the vertex buffer
            // to the input elements in a vertex shader
            // (by using so-called "semantic" names so that, for example,
            // "POSITION" here matches with "POSITION" in shader code.
            // "COLOR" here matches with "COLOR" in shader code).
            // Note that OpenGL uses arbitrarily assignable number IDs to do the same thing.
            constexpr unsigned int vertexElementCount = 2;
            D3D11_INPUT_ELEMENT_DESC layoutDescription[vertexElementCount] = {};
            {
                // Slot 0

                // POSITION
                // 3 floats == 12 bytes
                // Offset = 0
                {
                    auto& positionElement = layoutDescription[0];

                    positionElement.SemanticName = "POSITION";
                    positionElement.SemanticIndex = 0;  // (Semantics without modifying indices at the end can always use zero)
                    positionElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                    positionElement.InputSlot = 0;
                    positionElement.AlignedByteOffset = offsetof(eae6320::Graphics::VertexFormats::sMesh, x);
                    positionElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    positionElement.InstanceDataStepRate = 0;   // (Must be zero for per-vertex data)
                }

                // TEXCOORD
                // 2 floats == 8 bytes
                // Offset = 12
                {
                    auto& colorElement = layoutDescription[1];

                    colorElement.SemanticName = "TEXCOORD";
                    colorElement.SemanticIndex = 0; // (Semantics without modifying indices at the end can always use zero)
                    colorElement.Format = DXGI_FORMAT_R32G32_FLOAT;
                    colorElement.InputSlot = 0;
                    colorElement.AlignedByteOffset = offsetof(eae6320::Graphics::VertexFormats::sMesh, u);
                    colorElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    colorElement.InstanceDataStepRate = 0;  // (Must be zero for per-vertex data)
                }
            }

            const auto d3dResult = direct3dDevice->CreateInputLayout(layoutDescription, vertexElementCount,
                vertexShaderDataFromFile.data, vertexShaderDataFromFile.size, &m_vertexInputLayout);
            if (FAILED(result))
            {
                result = eae6320::Results::Failure;
                EAE6320_ASSERTF(false, "Geometry vertex input layout creation failed (HRESULT %#010x)", d3dResult);
                eae6320::Logging::OutputError("Direct3D failed to create the geometry vertex input layout (HRESULT %#010x)", d3dResult);
            }

            vertexShaderDataFromFile.Free();
        }
        else
        {
            EAE6320_ASSERTF(false, errorMessage.c_str());
            eae6320::Logging::OutputError("The geometry vertex input layout shader couldn't be loaded: %s", errorMessage.c_str());
            goto OnExit;
        }
    }

    // Vertex Buffer
    {
        // Allocate memory for the vertex data
        eae6320::Graphics::VertexFormats::sMesh* vertexData = static_cast<eae6320::Graphics::VertexFormats::sMesh*>(malloc(i_vertexCount * sizeof(eae6320::Graphics::VertexFormats::sMesh)));
        if (vertexData == nullptr)
        {
            result = Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Couldn't allocate memory for the mesh's vertex data!");
            Logging::OutputError("Failed to allocated memory for the mesh's vertex data!");
            goto OnExit;
        }
        GetVertexBufferData(vertexData, i_vertexCount, i_vertices, i_uvs);

        D3D11_BUFFER_DESC bufferDescription{};
        {
            const auto bufferSize = i_vertexCount * sizeof(eae6320::Graphics::VertexFormats::sMesh);
            EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(bufferDescription.ByteWidth) * 8)));
            bufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
            bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;    // In our class the buffer will never change after it's been created
            bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDescription.CPUAccessFlags = 0;   // No CPU access is necessary
            bufferDescription.MiscFlags = 0;
            bufferDescription.StructureByteStride = 0;  // Not used
        }
        D3D11_SUBRESOURCE_DATA initialData{};
        {
            initialData.pSysMem = vertexData;
            // (The other data members are ignored for non-texture buffers)
        }

        const auto d3dResult = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &m_vertexBuffer);
        if (FAILED(d3dResult))
        {
            result = eae6320::Results::Failure;
            EAE6320_ASSERTF(false, "Geometry vertex buffer creation failed (HRESULT %#010x)", d3dResult);
            eae6320::Logging::OutputError("Direct3D failed to create a geometry vertex buffer (HRESULT %#010x)", d3dResult);
            goto OnExit;
        }
    }

    // Index Buffer
    {
        // Allocate memory for the index data
        uint16_t* indexData = static_cast<uint16_t*>(malloc(i_indexCount * sizeof(uint16_t)));
        if (indexData == nullptr)
        {
            result = Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Couldn't allocate memory for the mesh's index data!");
            Logging::OutputError("Failed to allocated memory for the mesh's index data!");
            goto OnExit;
        }
        
        // Initialize the index buffer data
        {
            memcpy_s(indexData, i_indexCount * sizeof(uint16_t), i_indices, i_indexCount * sizeof(uint16_t));
            // D3D uses clockwise winding so swap the indices accordingly
            const uint16_t numTriangles = i_indexCount / s_indicesPerTriangle;
            for (uint16_t i = 0; i < numTriangles; ++i)
            {
                std::swap(indexData[i * s_indicesPerTriangle + 1], indexData[i * s_indicesPerTriangle + 2]);
            }
        }

        D3D11_BUFFER_DESC bufferDescription{};
        {
            const auto bufferSize = i_indexCount * sizeof(uint16_t);
            EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(bufferDescription.ByteWidth) * 8)));
            bufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
            bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;    // In our class the buffer will never change after it's been created
            bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bufferDescription.CPUAccessFlags = 0;   // No CPU access is necessary
            bufferDescription.MiscFlags = 0;
            bufferDescription.StructureByteStride = 0;  // Not used
        }
        D3D11_SUBRESOURCE_DATA initialData{};
        {
            initialData.pSysMem = indexData;
            // (The other data members are ignored for non-texture buffers)
        }

        const auto d3dResult = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &m_indexBuffer);

        // Free the memory allocated for the index data
        free(indexData);
        indexData = nullptr;

        if (FAILED(d3dResult))
        {
            result = eae6320::Results::Failure;
            EAE6320_ASSERTF(false, "Geometry index buffer creation failed (HRESULT %#010x)", d3dResult);
            eae6320::Logging::OutputError("Direct3D failed to create a geometry index buffer (HRESULT %#010x)", d3dResult);
            goto OnExit;
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::Graphics::cMesh::CleanUp()
{
    auto result = eae6320::Results::Success;

    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }

    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = nullptr;
    }

    if (m_vertexInputLayout)
    {
        m_vertexInputLayout->Release();
        m_vertexInputLayout = nullptr;
    }

    return result;
}
