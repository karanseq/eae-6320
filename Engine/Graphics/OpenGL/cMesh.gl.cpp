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

eae6320::cResult eae6320::Graphics::cMesh::Initialize(const uint16_t i_vertexCount, const Graphics::VertexFormats::sMesh* i_vertexData, const uint16_t i_indexCount, const uint16_t* i_indices)
{
    auto result = eae6320::Results::Success;

    // Save the vertex count as the index count
    m_indexCount = i_indexCount;

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
        const auto bufferSize = i_vertexCount * sizeof(eae6320::Graphics::VertexFormats::sMesh);
        EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(const_cast<VertexFormats::sMesh*>(i_vertexData)),
            // In our class we won't ever read from the buffer
            GL_STATIC_DRAW);

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
        const auto bufferSize = i_indexCount * sizeof(uint16_t);
        EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(const_cast<uint16_t*>(i_indices)),
            // In our class we won't ever read from the buffer
            GL_STATIC_DRAW);

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
        // 3 floats == 12 bytes
        // Offset = 0
        {
            constexpr GLuint vertexElementLocation = 0;
            constexpr GLint elementCount = 3;
            constexpr GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
            glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride,
                reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sMesh, x)));
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

        // Texture Coordinates (1)
        // 2 floats == 8 bytes
        // Offset = 12
        {
            constexpr GLuint vertexElementLocation = 1;
            constexpr GLint elementCount = 2;
            constexpr GLboolean normalized = GL_FALSE;  // The given floats should be used as-is
            glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, normalized, stride,
                reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sMesh, u)));
            const auto errorCode = glGetError();
            if (errorCode == GL_NO_ERROR)
            {
                glEnableVertexAttribArray(vertexElementLocation);
                const GLenum errorCode = glGetError();
                if (errorCode != GL_NO_ERROR)
                {
                    result = eae6320::Results::Failure;
                    EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                    eae6320::Logging::OutputError("OpenGL failed to enable the TEXCOORD vertex attribute at location %u: %s",
                        vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                    goto OnExit;
                }
            }
            else
            {
                result = eae6320::Results::Failure;
                EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
                eae6320::Logging::OutputError("OpenGL failed to set the TEXCOORD vertex attribute at location %u: %s",
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
