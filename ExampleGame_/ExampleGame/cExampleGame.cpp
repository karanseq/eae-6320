// Include Files
//==============

#include "cExampleGame.h"
#include "cGameObject.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cSprite.h>
#include <Engine/Graphics/Graphics.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/sVector.h>
#include <Engine/Math/sVector2d.h>
#include <Engine/UserInput/UserInput.h>
#include <Engine/UserInterface/cWidget.h>

#include <ctime>
#include <string>

// Static Data Initialization
//===========================

const std::string eae6320::cExampleGame::s_meshVertexShaderFilePath("data/Shaders/Vertex/mesh.shd");
const std::string eae6320::cExampleGame::s_meshFragmentShaderFilePath("data/Shaders/Fragment/mesh.shd");

// Inherited Implementation
//=========================

// Run
//----

void eae6320::cExampleGame::UpdateBasedOnInput()
{
    // Is the user pressing the ESC key?
    if (UserInput::IsKeyPressed(UserInput::KeyCodes::Escape))
    {
        // Exit the application
        const auto result = Exit( EXIT_SUCCESS );
        EAE6320_ASSERT(result);
    }
}

void eae6320::cExampleGame::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{

}

void eae6320::cExampleGame::UpdateSimulationBasedOnInput()
{
    // update camera
    {
        static const float cameraImpulseMagnitude = 2.0f;
        Math::sVector impulse;
        impulse.x = UserInput::IsKeyPressed('A') ? -cameraImpulseMagnitude : 0.0f + UserInput::IsKeyPressed('D') ? cameraImpulseMagnitude : 0.0f;
        impulse.y = UserInput::IsKeyPressed('E') ? -cameraImpulseMagnitude : 0.0f + UserInput::IsKeyPressed('Q') ? cameraImpulseMagnitude : 0.0f;
        impulse.z = UserInput::IsKeyPressed('W') ? -cameraImpulseMagnitude : 0.0f + UserInput::IsKeyPressed('S') ? cameraImpulseMagnitude : 0.0f;

        m_camera.m_rigidBodyState.velocity += impulse;
        m_camera.m_rigidBodyState.velocity.x = eae6320::Math::Clamp<float>(m_camera.m_rigidBodyState.velocity.x, -cGameObject::s_maxVelocity, cGameObject::s_maxVelocity);
        m_camera.m_rigidBodyState.velocity.y = eae6320::Math::Clamp<float>(m_camera.m_rigidBodyState.velocity.y, -cGameObject::s_maxVelocity, cGameObject::s_maxVelocity);
        m_camera.m_rigidBodyState.velocity.z = eae6320::Math::Clamp<float>(m_camera.m_rigidBodyState.velocity.z, -cGameObject::s_maxVelocity, cGameObject::s_maxVelocity);
    }

    // update game objects
    {
        static const float gameObjectImpulseMagnitude = 1.5f;
        Math::sVector impulse;
        impulse.x += UserInput::IsKeyPressed(UserInput::KeyCodes::Left) ? -gameObjectImpulseMagnitude : 0.0f + UserInput::IsKeyPressed(UserInput::KeyCodes::Right) ? gameObjectImpulseMagnitude : 0.0f;
        impulse.y = UserInput::IsKeyPressed(UserInput::KeyCodes::Down) ? -gameObjectImpulseMagnitude : 0.0f + UserInput::IsKeyPressed(UserInput::KeyCodes::Up) ? gameObjectImpulseMagnitude : 0.0f;

        m_gameObjectList[0]->AddImpulse(impulse);
    }
}

void eae6320::cExampleGame::UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
    UpdateGameObjects(i_elapsedSecondCount_sinceLastUpdate);
}

void eae6320::cExampleGame::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
    Graphics::SubmitBackgroundColor(m_backgroundColor);
    Graphics::SubmitDepthToClear();

    {
        const Math::cQuaternion predictedOrientation = m_camera.m_rigidBodyState.PredictFutureOrientation(i_elapsedSecondCount_sinceLastSimulationUpdate);
        const Math::sVector predictedPosition = m_camera.m_rigidBodyState.PredictFuturePosition(i_elapsedSecondCount_sinceLastSimulationUpdate);
        Graphics::SubmitCamera(m_camera, predictedPosition, predictedOrientation);
    }

    for (auto& gameObject : m_gameObjectList)
    {
        gameObject->SubmitDataToBeRendered(i_elapsedSecondCount_systemTime, i_elapsedSecondCount_sinceLastSimulationUpdate);
    }
}

void eae6320::cExampleGame::UpdateGameObjects(const float i_elapsedSecondCount_sinceLastUpdate)
{
    {
        // Apply drag
        m_camera.m_rigidBodyState.velocity.x -= fabsf(m_camera.m_rigidBodyState.velocity.x) > 0.0f ? m_camera.m_rigidBodyState.velocity.x * cGameObject::s_linearDamping : 0.0f;
        m_camera.m_rigidBodyState.velocity.y -= fabsf(m_camera.m_rigidBodyState.velocity.y) > 0.0f ? m_camera.m_rigidBodyState.velocity.y * cGameObject::s_linearDamping : 0.0f;
        m_camera.m_rigidBodyState.velocity.z -= fabsf(m_camera.m_rigidBodyState.velocity.z) > 0.0f ? m_camera.m_rigidBodyState.velocity.z * cGameObject::s_linearDamping : 0.0f;
        
        m_camera.m_rigidBodyState.Update(i_elapsedSecondCount_sinceLastUpdate);
    }

    for (const auto& gameObject : m_gameObjectList)
    {
        gameObject->UpdateBasedOnTime(i_elapsedSecondCount_sinceLastUpdate);
    }
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::cExampleGame::Initialize()
{
    cResult result = Results::Success;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Initialize effects
    if (!(result = InitializeEffects()))
    {
        goto OnExit;
    }

    // Initialize textures
    if (!(result = InitializeTextures()))
    {
        goto OnExit;
    }

    // Initialize meshes
    if (!(result = InitializeMeshes()))
    {
        goto OnExit;
    }

    // Initialize game objects
    if (!(result = InitializeGameObjects()))
    {
        goto OnExit;
    }

    m_camera.m_rigidBodyState.position.y = 2.5f;
    m_camera.m_rigidBodyState.position.z = 15.0f;

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::CleanUp()
{
    cResult result = Results::Success;

    for (const auto& effect : m_effectList)
    {
        effect->DecrementReferenceCount();
    }
    m_effectList.clear();

    for (auto& texture : m_textureList)
    {
        const auto localResult = Graphics::cTexture::s_manager.Release(texture);
        if (!localResult)
        {
            EAE6320_ASSERT(false);
            if (result)
            {
                result = localResult;
            }
        }
    }
    m_textureList.clear();

    for (auto& gameObject : m_gameObjectList)
    {
        cGameObject::Destroy(gameObject);
    }
    m_gameObjectList.clear();

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeEffects()
{
    cResult result = Results::Success;

    constexpr uint8_t numEffects = 1;
    m_effectList.reserve(numEffects);

    // initialize the mesh effect
    {
        Graphics::cEffect* effect = nullptr;

        if (!(result = Graphics::cEffect::Create(effect, s_meshVertexShaderFilePath.c_str(), s_meshFragmentShaderFilePath.c_str(), Graphics::RenderStates::DepthBuffering)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_effectList.push_back(effect);
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeTextures()
{
    cResult result = Results::Success;

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeMeshes()
{
    cResult result = Results::Success;

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeGameObjects()
{
    cResult result = Results::Success;

    cGameObject* gameObject = nullptr;

    if (!(result = cGameObject::Create(gameObject, Math::sVector(0.0f, -2.5f, 0.0f))))
    {
        EAE6320_ASSERT(false);
        goto OnExit;
    }
    else
    {
        m_gameObjectList.push_back(gameObject);
    }

OnExit:

    return result;
}

void eae6320::cExampleGame::GetRandomOriginForSprite(eae6320::Math::sVector2d& o_origin) const
{
    o_origin.x = float(std::rand() % 20000) * 0.0001f - 1.0f;
    o_origin.y = float(std::rand() % 20000) * 0.0001f - 1.0f;
}

void eae6320::cExampleGame::GetRandomExtentsForSprite(eae6320::Math::sVector2d& o_extents) const
{
    static const float minWidthHeight = 0.05f;
    static const float maxWidthHeight = 0.2f;

    o_extents.x = minWidthHeight + maxWidthHeight * float(std::rand() % 10000) * 0.0001f;
    o_extents.y = minWidthHeight + maxWidthHeight * float(std::rand() % 10000) * 0.0001f;
}
