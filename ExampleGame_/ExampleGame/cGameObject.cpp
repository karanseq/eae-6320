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

eae6320::cResult eae6320::cGameObject::Create(cGameObject*& o_gameObject, const Math::sVector& i_position)
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
    if (!(result = newGameObject->Initialize(i_position)))
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

eae6320::cResult eae6320::cGameObject::Initialize(const Math::sVector& i_position)
{
    auto result = Results::Success;

    // Save the position
    m_rigidBodyState.position = i_position;

    // Initialize the effect
    {
        if (!(result = Graphics::cEffect::Create(m_effect, cExampleGame::s_meshVertexShaderFilePath.c_str(), cExampleGame::s_meshFragmentShaderFilePath.c_str(), Graphics::RenderStates::DepthBuffering)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
    }

    // Initialize the mesh
    {
        if (!(result = Graphics::cMesh::s_manager.Load("data/Meshes/SoccerBall.msh", m_mesh)))
        {
            EAE6320_ASSERTF(false, "Could not initialize the mesh for game object!");
            goto OnExit;
        }
    }

    // Initialize the texture
    {
        if (!(result = Graphics::cTexture::s_manager.Load("data/Textures/Soccer/SoccerBall.tex", m_texture)))
        {
            EAE6320_ASSERTF(false, "Could not initialize the texture for game object!");
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

    Graphics::cTexture::s_manager.Release(m_texture);
    Graphics::cMesh::s_manager.Release(m_mesh);

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
    
    Graphics::cMesh* mesh = Graphics::cMesh::s_manager.Get(m_mesh);
    Graphics::cTexture* texture = Graphics::cTexture::s_manager.Get(m_texture);
    
    Graphics::SubmitMeshToBeRendered(mesh, m_effect, texture, predictedPosition, predictedOrientation);
}
