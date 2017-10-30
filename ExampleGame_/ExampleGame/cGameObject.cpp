// Include Files
//==============

#include "cGameObject.h"
#include "cExampleGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/Graphics.h>
#include <Engine/Graphics/sColor.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/Functions.h>
#include <Engine/Math/sVector.h>

#include <new>

// Static Data Initialization
//===========================
const float eae6320::cGameObject::s_maxVelocity = 2.5f;
const float eae6320::cGameObject::s_linearDamping = 0.1f;

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::cGameObject::Create(cGameObject*& o_gameObject, const Math::sVector& i_position, const Graphics::sColor& i_innerColor, const Graphics::sColor& i_outerColor)
{
    auto result = Results::Success;

    cGameObject* newGameObject = nullptr;

    // Allocate a new game object
    {
        newGameObject = new (std::nothrow) cGameObject();
        if (newGameObject == nullptr)
        {
            result = Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Couldn't allocate memory for the new game object!");
            Logging::OutputError("Failed to allocated memory for the new game object!");
            goto OnExit;
        }
    }

    // Initialize the new game object
    if (!(result = newGameObject->Initialize(i_position, i_innerColor, i_outerColor)))
    {
        EAE6320_ASSERTF(false, "Could not initialize the new game object!");
        goto OnExit;
    }

OnExit:

    if (result)
    {
        EAE6320_ASSERT(newGameObject);
        o_gameObject = newGameObject;
    }
    else
    {
        if (newGameObject)
        {
            Destroy(newGameObject);
        }
        o_gameObject = nullptr;
    }

    return result;
}

eae6320::cResult eae6320::cGameObject::Destroy(cGameObject*& i_gameObject)
{
    EAE6320_ASSERT(i_gameObject);

    delete i_gameObject;
    i_gameObject = nullptr;

    return Results::Success;
}

eae6320::cResult eae6320::cGameObject::Initialize(const Math::sVector& i_position, const Graphics::sColor& i_innerColor, const Graphics::sColor& i_outerColor)
{
    auto result = Results::Success;

    // Save the position
    m_rigidBodyState.position = i_position;

    // Initialize the effect
    {
        if (!(result = eae6320::Graphics::cEffect::Create(m_effect, cExampleGame::s_meshVertexShaderFilePath.c_str(), cExampleGame::s_meshFragmentShaderFilePath.c_str(), Graphics::RenderStates::AlphaTransparency | Graphics::RenderStates::DepthBuffering)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
    }

    // Initialize the mesh
    {
        constexpr uint16_t vertexCount = 36;
        
        const Math::sVector vertices[vertexCount] = { 
            { -1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f },            // front-down
            { -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, 1.0f },            // front-up

            { -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, 1.0f }, { -1.0f, 1.0f, 1.0f },         // left-down
            { -1.0f, -1.0f, -1.0f }, { -1.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, -1.0f },         // left-up

            { 1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, -1.0f },           // right-down
            { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f },             // right-up

            { 1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f }, { -1.0f, 1.0f, -1.0f },        // back-down
            { 1.0f, -1.0f, -1.0f }, { -1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, -1.0f },          // back-up

            { -1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, -1.0f },             // top-down
            { -1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, -1.0f }, { -1.0f, 1.0f, -1.0f },           // top-up

            { -1.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, 1.0f },         // bottom-down
            { -1.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, 1.0f }          // bottom-up
        };

        uint16_t indices[vertexCount];
        for (uint16_t i = 0; i < vertexCount; ++i)
        {
            indices[i] = i;
        }

        const Graphics::sColor colors[vertexCount] = {
            Graphics::sColor::RED, Graphics::sColor::PURPLE, Graphics::sColor::BLUE,        // front-down
            Graphics::sColor::RED, Graphics::sColor::BLUE, Graphics::sColor::PURPLE,        // front-up

            Graphics::sColor::RED, Graphics::sColor::RED, Graphics::sColor::PURPLE,         // left-down
            Graphics::sColor::RED, Graphics::sColor::PURPLE, Graphics::sColor::PURPLE,      // left-up

            Graphics::sColor::PURPLE, Graphics::sColor::PURPLE, Graphics::sColor::BLUE,     // right-down
            Graphics::sColor::PURPLE, Graphics::sColor::BLUE, Graphics::sColor::BLUE,       // right-up

            Graphics::sColor::PURPLE, Graphics::sColor::RED, Graphics::sColor::PURPLE,      // back-down
            Graphics::sColor::PURPLE, Graphics::sColor::PURPLE, Graphics::sColor::BLUE,     // back-up

            Graphics::sColor::PURPLE, Graphics::sColor::BLUE, Graphics::sColor::BLUE,       // top-down
            Graphics::sColor::PURPLE, Graphics::sColor::BLUE, Graphics::sColor::PURPLE,     // top-up

            Graphics::sColor::RED, Graphics::sColor::PURPLE, Graphics::sColor::PURPLE,      // bottom-down
            Graphics::sColor::RED, Graphics::sColor::PURPLE, Graphics::sColor::RED,         // bottom-up
        };

        if (!(result = eae6320::Graphics::cMesh::Create(m_mesh, vertexCount, vertices, indices, colors)))
        {
            EAE6320_ASSERTF(false, "Could not initialize the new mesh!");
            goto OnExit;
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::cGameObject::CleanUp()
{
    m_effect->DecrementReferenceCount();
    m_effect = nullptr;

    m_mesh->DecrementReferenceCount();
    m_mesh = nullptr;

    return eae6320::Results::Success;
}

eae6320::cGameObject::~cGameObject()
{
    CleanUp();
}

// Behavior
//---------

void eae6320::cGameObject::AddImpulse(const Math::sVector& i_impulse)
{
    m_rigidBodyState.velocity += i_impulse;
    m_rigidBodyState.velocity.x = eae6320::Math::Clamp<float>(m_rigidBodyState.velocity.x, -s_maxVelocity, s_maxVelocity);
    m_rigidBodyState.velocity.y = eae6320::Math::Clamp<float>(m_rigidBodyState.velocity.y, -s_maxVelocity, s_maxVelocity);
    m_rigidBodyState.velocity.z = eae6320::Math::Clamp<float>(m_rigidBodyState.velocity.z, -s_maxVelocity, s_maxVelocity);
}

// Update
//-------

void eae6320::cGameObject::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
    // Apply drag
    m_rigidBodyState.velocity.x -= fabsf(m_rigidBodyState.velocity.x) > 0.0f ? m_rigidBodyState.velocity.x * s_linearDamping : 0.0f;
    m_rigidBodyState.velocity.y -= fabsf(m_rigidBodyState.velocity.y) > 0.0f ? m_rigidBodyState.velocity.y * s_linearDamping : 0.0f;
    m_rigidBodyState.velocity.z -= fabsf(m_rigidBodyState.velocity.z) > 0.0f ? m_rigidBodyState.velocity.z * s_linearDamping : 0.0f;

    m_rigidBodyState.Update(i_elapsedSecondCount_sinceLastUpdate);
}

// Render
//-------

void eae6320::cGameObject::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
    const Math::cQuaternion predictedOrientation = m_rigidBodyState.PredictFutureOrientation(i_elapsedSecondCount_sinceLastSimulationUpdate);
    const Math::sVector predictedPosition = m_rigidBodyState.PredictFuturePosition(i_elapsedSecondCount_sinceLastSimulationUpdate);
    eae6320::Graphics::SubmitMeshToBeRendered(m_mesh, m_effect, predictedPosition, predictedOrientation);
}
