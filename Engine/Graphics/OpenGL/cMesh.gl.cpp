// Include Files
//==============

#include "../cMesh.h"

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
    // Bind the vertex array to the device as a data source
    {
        glBindVertexArray(m_vertexArrayId);
        EAE6320_ASSERT(glGetError() == GL_NO_ERROR)
    }
    // Render triangles from the currently-bound vertex array
    {
        // The mode defines how to interpret multiple vertices as a single "primitive";
        // a triangle list is defined
        // (meaning that every primitive is a triangle and will be defined by three vertices)
        constexpr GLenum mode = GL_TRIANGLES;
        // It's possible to start rendering primitives in the middle of the steam
        const GLvoid* const offset = 0;
        glDrawElements(mode, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_SHORT, offset);
        EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
    }
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cMesh::Initialize(const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const eae6320::Graphics::sColor* i_colors)
{
    auto result = eae6320::Results::Success;

    // Save the vertex count as the index count
    m_indexCount = i_vertexCount;

    // Create a vertex array object and make it active
    {
        constexpr GLsizei arrayCount = 1;
        glGenVertexArrays(arrayCount, &m_vertexArrayId);
        const auto errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindVertexArray(m_vertexArrayId);
            const auto errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                result = eae6320::Results::Failure;
                EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                eae6320::Logging::OutputError("OpenGL failed to bind a new vertex array: %s",
                    reinterpret_cast<const char*>(gluErrorString(errorCode)));
                goto OnExit;
            }
        }
        else
        {
            result = eae6320::Results::Failure;
            EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
            eae6320::Logging::OutputError("OpenGL failed to get an unused vertex array ID: %s",
                reinterpret_cast<const char*>(gluErrorString(errorCode)));
            goto OnExit;
        }
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &m_vertexBufferId);
        const auto errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
            const auto errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                result = eae6320::Results::Failure;
                EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                eae6320::Logging::OutputError("OpenGL failed to bind a new vertex buffer: %s",
                    reinterpret_cast<const char*>(gluErrorString(errorCode)));
                goto OnExit;
            }
        }
        else
        {
            result = eae6320::Results::Failure;
            EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
            eae6320::Logging::OutputError("OpenGL failed to get an unused vertex buffer ID: %s",
                reinterpret_cast<const char*>(gluErrorString(errorCode)));
            goto OnExit;
        }
    }

    // Assign the data to the vertex buffer
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
        GetVertexBufferData(vertexData, i_vertexCount, i_vertices, i_colors);

        const auto bufferSize = i_vertexCount * sizeof(eae6320::Graphics::VertexFormats::sMesh);
        EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(vertexData),
            // In our class we won't ever read from the buffer
            GL_STATIC_DRAW);

        // Free the memory allocated for the vertex data
        free(vertexData);
        vertexData = nullptr;

        const auto errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            result = eae6320::Results::Failure;
            EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
            eae6320::Logging::OutputError("OpenGL failed to allocate the vertex buffer: %s",
                reinterpret_cast<const char*>(gluErrorString(errorCode)));
            goto OnExit;
        }
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &m_indexBufferId);
        const auto errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
            const auto errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                result = eae6320::Results::Failure;
                EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                eae6320::Logging::OutputError("OpenGL failed to bind a new vertex buffer: %s",
                    reinterpret_cast<const char*>(gluErrorString(errorCode)));
                goto OnExit;
            }
        }
        else
        {
            result = eae6320::Results::Failure;
            EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
            eae6320::Logging::OutputError("OpenGL failed to get an unused vertex buffer ID: %s",
                reinterpret_cast<const char*>(gluErrorString(errorCode)));
            goto OnExit;
        }
    }

    // Assign data to the index buffer
    {
        // Allocate memory for the index data
        uint16_t* indexData = static_cast<uint16_t*>(malloc(i_vertexCount * sizeof(uint16_t)));
        if (indexData == nullptr)
        {
            result = Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Couldn't allocate memory for the mesh's index data!");
            Logging::OutputError("Failed to allocated memory for the mesh's index data!");
            goto OnExit;
        }
        GetIndexBufferData(indexData, i_vertexCount, i_vertices);

        const auto bufferSize = i_vertexCount * sizeof(uint16_t);
        EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(indexData),
            // In our class we won't ever read from the buffer
            GL_STATIC_DRAW);

        // Free the memory allocated for the index data
        free(indexData);
        indexData = nullptr;

        const auto errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            result = eae6320::Results::Failure;
            EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
            eae6320::Logging::OutputError("OpenGL failed to allocate the index buffer: %s",
                reinterpret_cast<const char*>(gluErrorString(errorCode)));
            goto OnExit;
        }
    }

    // Initialize vertex format
    {
        // The "stride" defines how large a single vertex is in the stream of data
        // (or, said another way, how far apart each position element is)
        const auto stride = static_cast<GLsizei>(sizeof(eae6320::Graphics::VertexFormats::sMesh));

        // Position (0)
        // 2 floats == 8 bytes
        // Offset = 0
        {
            constexpr GLuint vertexElementLocation = 0;
            constexpr GLint elementCount = 2;
            constexpr GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
            glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride,
                reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sSprite, x)));
            const auto errorCode = glGetError();
            if (errorCode == GL_NO_ERROR)
            {
                glEnableVertexAttribArray(vertexElementLocation);
                const GLenum errorCode = glGetError();
                if (errorCode != GL_NO_ERROR)
                {
                    result = eae6320::Results::Failure;
                    EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                    eae6320::Logging::OutputError("OpenGL failed to enable the POSITION vertex attribute at location %u: %s",
                        vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                    goto OnExit;
                }
            }
            else
            {
                result = eae6320::Results::Failure;
                EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                eae6320::Logging::OutputError("OpenGL failed to set the POSITION vertex attribute at location %u: %s",
                    vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                goto OnExit;
            }
        }

        // Color (1)
        // 4 8-bit ints == 4 bytes
        // Offset = 8
        {
            constexpr GLuint vertexElementLocation = 1;
            constexpr GLint elementCount = 4;
            constexpr GLboolean normalized = GL_TRUE;
            glVertexAttribPointer(vertexElementLocation, elementCount, GL_UNSIGNED_BYTE, normalized, stride,
                reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sMesh, r)));
            const auto errorCode = glGetError();
            if (errorCode == GL_NO_ERROR)
            {
                glEnableVertexAttribArray(vertexElementLocation);
                const GLenum errorCode = glGetError();
                if (errorCode != GL_NO_ERROR)
                {
                    result = eae6320::Results::Failure;
                    EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                    eae6320::Logging::OutputError("OpenGL failed to enable the COLOR vertex attribute at location %u: %s",
                        vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                    goto OnExit;
                }
            }
            else
            {
                result = eae6320::Results::Failure;
                EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                eae6320::Logging::OutputError("OpenGL failed to set the COLOR vertex attribute at location %u: %s",
                    vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                goto OnExit;
            }
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::Graphics::cMesh::CleanUp()
{
    auto result = eae6320::Results::Success;

    if (m_vertexArrayId != 0)
    {
        // Make sure that the vertex array isn't bound
        {
            // Unbind the vertex array
            glBindVertexArray(0);
            const auto errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                if (result)
                {
                    result = Results::Failure;
                }
                EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                Logging::OutputError("OpenGL failed to unbind all vertex arrays before cleaning up geometry: %s",
                    reinterpret_cast<const char*>(gluErrorString(errorCode)));
            }
        }
        constexpr GLsizei arrayCount = 1;
        glDeleteVertexArrays(arrayCount, &m_vertexArrayId);
        const auto errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            if (result)
            {
                result = Results::Failure;
            }
            EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
            Logging::OutputError("OpenGL failed to delete the vertex array: %s",
                reinterpret_cast<const char*>(gluErrorString(errorCode)));
        }
        m_vertexArrayId = 0;
    }
    if (m_vertexBufferId != 0)
    {
        constexpr GLsizei bufferCount = 1;
        glDeleteBuffers(bufferCount, &m_vertexBufferId);
        const auto errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            if (result)
            {
                result = Results::Failure;
            }
            EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
            Logging::OutputError("OpenGL failed to delete the vertex buffer: %s",
                reinterpret_cast<const char*>(gluErrorString(errorCode)));
        }
        m_vertexBufferId = 0;
    }
    if (m_indexBufferId)
    {
        constexpr GLsizei bufferCount = 1;
        glDeleteBuffers(bufferCount, &m_indexBufferId);
        const auto errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            if (result)
            {
                result = Results::Failure;
            }
            EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
            Logging::OutputError("OpenGL failed to delete the index buffer: %s",
                reinterpret_cast<const char*>(gluErrorString(errorCode)));
        }
        m_indexBufferId = 0;
    }

    return result;
}
