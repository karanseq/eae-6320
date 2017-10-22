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

#include <new>

// Static Data Initialization
//===========================
const float eae6320::cGameObject::s_maxVelocityLengthSquared = 25.0f;
const float eae6320::cGameObject::s_linearDamping = 1.0f;

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
    m_position = i_position;

    // Initialize the effect
    {
        if (!(result = eae6320::Graphics::cEffect::Create(m_effect, cExampleGame::s_meshVertexShaderFilePath.c_str(), cExampleGame::s_meshFragmentShaderFilePath.c_str())))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
    }

    // Initialize the mesh
    {
        constexpr uint16_t vertexCount = 18;

        // Initialize vertices
        const Math::sVector vertices[vertexCount] = {
            { 0.0f, 0.0f, 0.0f }, { 0.25f, 0.0f, 0.0f }, { 0.25f, 0.25f, 0.0f },            // center-bottom
            { 0.0f, 0.0f, 0.0f }, { 0.25f, 0.25f, 0.0f }, { 0.0f, 0.25f, 0.0f },            // center-top
            { 0.0f, 0.25f, 0.0f }, { 0.25f, 0.25f, 0.0f }, { 0.125f, 0.5f, 0.0f },          // top
            { 0.0f, 0.0f, 0.0f }, { 0.125f, -0.25f, 0.0f }, { 0.25f, 0.0f, 0.0f },          // bottom
            { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.25f, 0.0f }, { -0.25f, 0.125f, 0.0f },          // left
            { 0.25f, 0.0f, 0.0f }, { 0.5f, 0.125f, 0.0f }, { 0.25f, 0.25f, 0.0f },          // right
        };

        // Initialize indices
        uint16_t indices[vertexCount];
        for (uint16_t i = 0; i < vertexCount; ++i)
        {
            indices[i] = i;
        }

        // Initilize colors
        const Graphics::sColor colors[vertexCount] = {
            i_innerColor, i_innerColor, i_innerColor,
            i_innerColor, i_innerColor, i_innerColor,
            i_innerColor, i_innerColor, i_outerColor,
            i_innerColor, i_outerColor, i_innerColor,
            i_innerColor, i_innerColor, i_outerColor,
            i_innerColor, i_outerColor, i_innerColor
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

// Update
//-------

void eae6320::cGameObject::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{

}

// Render
//-------

void eae6320::cGameObject::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
    eae6320::Graphics::SubmitMeshToBeRendered(m_mesh, m_effect, m_position);
}
