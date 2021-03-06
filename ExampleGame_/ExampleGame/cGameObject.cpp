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
#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/Functions.h>
#include <Engine/Math/sVector.h>

#include <new>

// Static Data Initialization
//===========================
const float eae6320::cGameObject::s_linearDamping = 0.1f;
const float eae6320::cGameObject::s_angularDamping = eae6320::Math::Pi * 0.05f;

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::cGameObject::Create(cGameObject*& o_gameObject, const sGameObjectinitializationParameters& i_initializationParameters)
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
    if (!(result = newGameObject->Initialize(i_initializationParameters)))
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

eae6320::cResult eae6320::cGameObject::Initialize(const sGameObjectinitializationParameters& i_initializationParameters)
{
    auto result = Results::Success;

    // Validate initialization parameters
    if (!i_initializationParameters.IsValid())
    {
        result = Results::Failure;
        goto OnExit;
    }

    // Save physics information
    {
        m_rigidBodyState.position = i_initializationParameters.initialPosition;
        m_maxVelocityLengthSquared = i_initializationParameters.maxVelocity * i_initializationParameters.maxVelocity;
        m_angularSpeed = i_initializationParameters.angularSpeed;
        m_maxAngularSpeed = i_initializationParameters.maxAngularSpeed;
        m_linearDamping = i_initializationParameters.linearDamping;
        m_angularDamping = i_initializationParameters.angularDamping;
    }

    // Initialize the effect
    {
        if (!(result = Graphics::cEffect::Create(m_effect, i_initializationParameters.vertexShaderFilePath->c_str(), i_initializationParameters.fragmentShaderFilePath->c_str(), Graphics::RenderStates::DepthBuffering)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
    }

    // Initialize the mesh
    {
        if (!(result = Graphics::cMesh::s_manager.Load(i_initializationParameters.meshFilePath->c_str(), m_mesh)))
        {
            EAE6320_ASSERTF(false, "Could not initialize the mesh for game object!");
            goto OnExit;
        }
    }

    // Initialize the texture
    {
        if (!(result = Graphics::cTexture::s_manager.Load(i_initializationParameters.textureFilePath->c_str(), m_texture)))
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
    const Math::sVector impulse = Math::cMatrix_transformation(m_rigidBodyState.orientation, Math::sVector::ZERO) * i_impulse;
    m_rigidBodyState.velocity += m_rigidBodyState.velocity.GetLengthSquared() < m_maxVelocityLengthSquared ? impulse : Math::sVector::ZERO;
}

void eae6320::cGameObject::AddYaw(float i_delta)
{
    m_angularImpulseReceived.y = i_delta;
}

void eae6320::cGameObject::AddPitch(float i_delta)
{
    m_angularImpulseReceived.x = i_delta;
}

// Update
//-------

void eae6320::cGameObject::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
    // Apply angular impulse
    if (fabsf(m_angularImpulseReceived.x) > 0.0f || fabsf(m_angularImpulseReceived.y))
    {
        m_rigidBodyState.angularVelocity_axis_local.x = m_angularImpulseReceived.x;
        m_rigidBodyState.angularVelocity_axis_local.y = m_angularImpulseReceived.y;
        m_rigidBodyState.angularSpeed += m_rigidBodyState.angularSpeed > m_maxAngularSpeed ? 0.0f : m_angularSpeed;
    }

    m_rigidBodyState.Update(i_elapsedSecondCount_sinceLastUpdate);

    // Apply linear damping
    m_rigidBodyState.velocity.x -= fabsf(m_rigidBodyState.velocity.x) > 0.0f ? m_rigidBodyState.velocity.x * m_linearDamping : 0.0f;
    m_rigidBodyState.velocity.y -= fabsf(m_rigidBodyState.velocity.y) > 0.0f ? m_rigidBodyState.velocity.y * m_linearDamping : 0.0f;
    m_rigidBodyState.velocity.z -= fabsf(m_rigidBodyState.velocity.z) > 0.0f ? m_rigidBodyState.velocity.z * m_linearDamping : 0.0f;

    // Apply angular damping
    m_rigidBodyState.angularSpeed -= fabsf(m_rigidBodyState.angularSpeed) > 0.0f ? m_rigidBodyState.angularSpeed * m_angularDamping : 0.0f;
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
