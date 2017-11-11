// Include Files
//==============

#include "../cSprite.h"

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

void eae6320::Graphics::cSprite::Draw() const
{
    auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
    EAE6320_ASSERT(direct3dImmediateContext);

    // Bind a specific vertex buffer to the device as a data source
    {
        EAE6320_ASSERT(m_vertexBuffer);
        constexpr unsigned int startingSlot = 0;
        constexpr unsigned int vertexBufferCount = 1;
        // The "stride" defines how large a single vertex is in the stream of data
        constexpr unsigned int bufferStride = sizeof(VertexFormats::sSprite);
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
        direct3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    }
    // Render triangles from the currently-bound vertex buffer
    {
        constexpr unsigned int vertexCountToRender = 4;
        // It's possible to start rendering primitives in the middle of the stream
        constexpr unsigned int indexOfFirstVertexToRender = 0;
        direct3dImmediateContext->Draw(vertexCountToRender, indexOfFirstVertexToRender);
    }
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cSprite::Initialize(const eae6320::Math::sVector2d& i_origin, const eae6320::Math::sVector2d& i_extents)
{
    auto result = eae6320::Results::Success;

    auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
    EAE6320_ASSERT(direct3dDevice);

    // Initialize vertex format
    {
        // Load the compiled binary vertex shader for the input layout
        eae6320::Platform::sDataFromFile vertexShaderDataFromFile;
        std::string errorMessage;
        if (result = eae6320::Platform::LoadBinaryFile("data/Shaders/Vertex/vertexInputLayout_sprite.shd", vertexShaderDataFromFile, &errorMessage))
        {
            // Create the vertex layout

            // These elements must match the VertexFormats::sSprite layout struct exactly.
            // They instruct Direct3D how to match the binary data in the vertex buffer
            // to the input elements in a vertex shader
            // (by using so-called "semantic" names so that, for example,
            // "POSITION" here matches with "POSITION" in shader code.
            // "TEXCOORD" here matches with "TEXCOORD" in shader code).
            // Note that OpenGL uses arbitrarily assignable number IDs to do the same thing.
            constexpr unsigned int vertexElementCount = 2;
            D3D11_INPUT_ELEMENT_DESC layoutDescription[vertexElementCount] = {};
            {
                // Slot 0

                // POSITION
                // 2 floats == 8 bytes
                // Offset = 0
                {
                    auto& positionElement = layoutDescription[0];

                    positionElement.SemanticName = "POSITION";
                    positionElement.SemanticIndex = 0;  // (Semantics without modifying indices at the end can always use zero)
                    positionElement.Format = DXGI_FORMAT_R32G32_FLOAT;
                    positionElement.InputSlot = 0;
                    positionElement.AlignedByteOffset = offsetof(eae6320::Graphics::VertexFormats::sSprite, x);
                    positionElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    positionElement.InstanceDataStepRate = 0;   // (Must be zero for per-vertex data)
                }

                // TEXCOORD
                // 2 floats == 8 bytes
                // Offset = 8
                {
                    auto& textureCoordElement = layoutDescription[1];

                    textureCoordElement.SemanticName = "TEXCOORD";
                    textureCoordElement.SemanticIndex = 0;  // (Semantics without modifying indices at the end can always use zero)
                    textureCoordElement.Format = DXGI_FORMAT_R32G32_FLOAT;
                    textureCoordElement.InputSlot = 0;
                    textureCoordElement.AlignedByteOffset = offsetof(eae6320::Graphics::VertexFormats::sSprite, u);
                    textureCoordElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    textureCoordElement.InstanceDataStepRate = 0;   // (Must be zero for per-vertex data)
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
        const auto vertexCount = 4;
        eae6320::Graphics::VertexFormats::sSprite vertexData[vertexCount];
        GetVertexPositions(vertexData, i_origin, i_extents);
        GetVertexTextureCoordinates(vertexData, i_origin, i_extents);

        {
            // D3D uses clockwise winding so swap the vertices appropriately
            std::swap(vertexData[0], vertexData[3]);
            // D3D uses top-to-bottom UVs so swap the Vs appropriately
            std::swap(vertexData[0].v, vertexData[2].v);
            std::swap(vertexData[1].v, vertexData[3].v);
        }

        D3D11_BUFFER_DESC bufferDescription{};
        {
            const auto bufferSize = vertexCount * sizeof(eae6320::Graphics::VertexFormats::sSprite);
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

OnExit:

    return result;
}

eae6320::cResult eae6320::Graphics::cSprite::CleanUp()
{
    auto result = eae6320::Results::Success;

    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }
    if (m_vertexInputLayout)
    {
        m_vertexInputLayout->Release();
        m_vertexInputLayout = nullptr;
    }

    return result;
}
