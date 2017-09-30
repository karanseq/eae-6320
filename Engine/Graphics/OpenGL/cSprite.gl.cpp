// Include Files
//==============

#include "../cSprite.h"

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
    // Bind a specific vertex buffer to the device as a data source
    {
        glBindVertexArray(m_vertexArrayId);
        EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
    }
    // Render triangles from the currently-bound vertex buffer
    {
        // The mode defines how to interpret multiple vertices as a single "primitive";
        // a triangle list is defined
        // (meaning that every primitive is a triangle and will be defined by three vertices)
        constexpr GLenum mode = GL_TRIANGLE_STRIP;
        // It's possible to start rendering primitives in the middle of the stream
        constexpr GLint indexOfFirstVertexToRender = 0;

        constexpr unsigned int vertexCountToRender = 4;
        glDrawArrays(mode, indexOfFirstVertexToRender, vertexCountToRender);
        EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
    }
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cSprite::Initialize(const eae6320::Math::sVector2d& i_origin, const eae6320::Math::sVector2d& i_extents)
{
    auto result = eae6320::Results::Success;

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
    // Assign the data to the buffer
    {
        const auto vertexCount = 4;
        eae6320::Graphics::VertexFormats::sSprite vertexData[vertexCount];
        GetVertexPositions(vertexData, i_origin, i_extents);
        GetVertexTextureCoordinates(vertexData, i_origin, i_extents);

        const auto bufferSize = vertexCount * sizeof(eae6320::Graphics::VertexFormats::sSprite);
        EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(GLsizeiptr) * 8)));
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(vertexData),
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
    // Initialize vertex format
    {
        // The "stride" defines how large a single vertex is in the stream of data
        // (or, said another way, how far apart each position element is)
        const auto stride = static_cast<GLsizei>(sizeof(eae6320::Graphics::VertexFormats::sSprite));

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

        // Texture Coordinates (1)
        // 2 floats == 8 bytes
        // Offset = 8
        {
            constexpr GLuint vertexElementLocation = 1;
            constexpr GLint elementCount = 2;
            constexpr GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
            glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride,
                reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sSprite, u)));
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

eae6320::cResult eae6320::Graphics::cSprite::CleanUp()
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

    return result;
}

void eae6320::Graphics::cSprite::GetVertexTextureCoordinates(eae6320::Graphics::VertexFormats::sSprite* o_vertexData, const eae6320::Math::sVector2d& i_origin, const eae6320::Math::sVector2d& i_extents) const
{
    EAE6320_ASSERT(o_vertexData);

    // bottom-right
    o_vertexData[0].u = 1;
    o_vertexData[0].v = 0;

    // top-right
    o_vertexData[1].u = 1;
    o_vertexData[1].v = 1;

    // bottom-left
    o_vertexData[2].u = 0;
    o_vertexData[2].v = 0;

    // top-left
    o_vertexData[3].u = 0;
    o_vertexData[3].v = 1;
}
