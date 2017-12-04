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
#include <Engine/Math/cMatrix_transformation.h>
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
        static constexpr float cameraImpulseMagnitude = 2.0f;
        Math::sVector impulse;
        impulse.x = UserInput::IsKeyPressed('A') ? -cameraImpulseMagnitude : 0.0f + UserInput::IsKeyPressed('D') ? cameraImpulseMagnitude : 0.0f;
        impulse.y = UserInput::IsKeyPressed('E') ? -cameraImpulseMagnitude : 0.0f + UserInput::IsKeyPressed('Q') ? cameraImpulseMagnitude : 0.0f;
        impulse.z = UserInput::IsKeyPressed('W') ? -cameraImpulseMagnitude : 0.0f + UserInput::IsKeyPressed('S') ? cameraImpulseMagnitude : 0.0f;

        impulse = Math::cMatrix_transformation(m_camera.m_rigidBodyState.orientation, Math::sVector::ZERO) * impulse;
        m_camera.m_rigidBodyState.velocity += impulse;

        m_camera.m_rigidBodyState.angularVelocity_axis_local.x = UserInput::IsKeyPressed('2') || UserInput::IsKeyPressed('4') ? 1.0f : 0.0f;
        m_camera.m_rigidBodyState.angularVelocity_axis_local.y = UserInput::IsKeyPressed('1') || UserInput::IsKeyPressed('3') ? 1.0f : 0.0f;

        static constexpr float cameraAngularSpeed = Math::Pi * 0.25f;
        m_camera.m_rigidBodyState.angularSpeed = UserInput::IsKeyPressed('1') || UserInput::IsKeyPressed('2') ? cameraAngularSpeed : 0.0f + UserInput::IsKeyPressed('3') || UserInput::IsKeyPressed('4') ? -cameraAngularSpeed : 0.0f;
    }

    // update game objects
    {
        static constexpr float gameObjectImpulseMagnitude = 2.0f;

        Math::sVector impulse;
        impulse.z = UserInput::IsKeyPressed(UserInput::KeyCodes::Space) ? -gameObjectImpulseMagnitude : 0.0f;
        m_gameObjectList[0]->AddImpulse(impulse);

        m_gameObjectList[0]->AddYaw(UserInput::IsKeyPressed(UserInput::KeyCodes::Left) ? 1.0f : 0.0f + UserInput::IsKeyPressed(UserInput::KeyCodes::Right) ? -1.0f : 0.0f);
        m_gameObjectList[0]->AddPitch(UserInput::IsKeyPressed(UserInput::KeyCodes::Up) ? 1.0f: 0.0f + UserInput::IsKeyPressed(UserInput::KeyCodes::Down) ? -1.0f : 0.0f);
    }
}

void eae6320::cExampleGame::UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
    //UpdateCoins(i_elapsedSecondCount_sinceLastUpdate);
    m_springArm.UpdateBasedOnTime(i_elapsedSecondCount_sinceLastUpdate);
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

        if (m_skyBoxEnabled)
        {
            static const Math::cQuaternion skyBoxOrientation(Math::Pi * 0.5f, Math::sVector(0.0f, 1.0f, 1.0f));

            Graphics::cMesh* mesh = Graphics::cMesh::s_manager.Get(m_skyBoxMesh);
            Graphics::cTexture* texture = Graphics::cTexture::s_manager.Get(m_skyBoxTexture);
            Graphics::SubmitMeshToBeRendered(mesh, m_skyBoxEffect, texture, predictedPosition, skyBoxOrientation);
        }
    }

    for (auto& gameObject : m_gameObjectList)
    {
        gameObject->SubmitDataToBeRendered(i_elapsedSecondCount_systemTime, i_elapsedSecondCount_sinceLastSimulationUpdate);
    }
}

void eae6320::cExampleGame::UpdateGameObjects(const float i_elapsedSecondCount_sinceLastUpdate)
{
    {
        m_camera.m_rigidBodyState.Update(i_elapsedSecondCount_sinceLastUpdate);

        // Apply drag
        m_camera.m_rigidBodyState.velocity.x -= fabsf(m_camera.m_rigidBodyState.velocity.x) > 0.0f ? m_camera.m_rigidBodyState.velocity.x * cGameObject::s_linearDamping : 0.0f;
        m_camera.m_rigidBodyState.velocity.y -= fabsf(m_camera.m_rigidBodyState.velocity.y) > 0.0f ? m_camera.m_rigidBodyState.velocity.y * cGameObject::s_linearDamping : 0.0f;
        m_camera.m_rigidBodyState.velocity.z -= fabsf(m_camera.m_rigidBodyState.velocity.z) > 0.0f ? m_camera.m_rigidBodyState.velocity.z * cGameObject::s_linearDamping : 0.0f;
    }

    for (const auto& gameObject : m_gameObjectList)
    {
        gameObject->UpdateBasedOnTime(i_elapsedSecondCount_sinceLastUpdate);
    }
}

void eae6320::cExampleGame::UpdateCoins(const float i_elapsedSecondCount_sinceLastUpdate)
{
    static constexpr float impulseZ = 2.5f;

    const size_t numGameObjects = m_gameObjectList.size();
    for (size_t i = 1; i < numGameObjects; ++i)
    {
        m_gameObjectList[i]->AddImpulse(Math::sVector(0.0f, 0.0f, impulseZ));
    }
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::cExampleGame::Initialize()
{
    cResult result = Results::Success;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Initialize game objects
    if (!(result = InitializeGameObjects()))
    {
        goto OnExit;
    }

    // Initialize the sky box
    if (m_skyBoxEnabled)
    {
        if (!(result = InitializeSkyBox()))
        {
            goto OnExit;
        }
    }

    // Initialize the camera
    {
        m_camera.m_rigidBodyState.angularVelocity_axis_local.x = 1.0f;
        m_camera.m_rigidBodyState.angularVelocity_axis_local.y = 0.0f;

        m_camera.m_rigidBodyState.position.y = 0.0f;
        m_camera.m_rigidBodyState.position.z = 15.0f;
    }

    // Initialize the spring arm
    {
        m_springArm.target = &m_gameObjectList[0]->GetRigidBodyState();
        m_springArm.camera = &m_camera.m_rigidBodyState;
        m_springArm.armLength = 10.0f;
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::CleanUp()
{
    cResult result = Results::Success;

    // Cleanup all game objects
    for (auto& gameObject : m_gameObjectList)
    {
        cGameObject::Destroy(gameObject);
    }
    m_gameObjectList.clear();

    // Cleanup sky box if enabled
    if (m_skyBoxEnabled)
    {
        {
            const auto localResult = Graphics::cMesh::s_manager.Release(m_skyBoxMesh);
            if (!localResult)
            {
                EAE6320_ASSERT(false);
                result = result ? localResult : result;
            }
        }

        {
            const auto localResult = Graphics::cTexture::s_manager.Release(m_skyBoxTexture);
            if (!localResult)
            {
                EAE6320_ASSERT(false);
                result = result ? localResult : result;
            }
        }

        {
            m_skyBoxEffect->DecrementReferenceCount();
            m_skyBoxEffect = nullptr;
        }
    }

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeGameObjects()
{
    cResult result = Results::Success;

    static const std::string meshFilePath = std::string("data/Meshes/Ship.msh");
    static const std::string textureFilePath = std::string("data/Textures/Ship.tex");

    sGameObjectinitializationParameters Params;
    Params.vertexShaderFilePath = &s_meshVertexShaderFilePath;
    Params.fragmentShaderFilePath = &s_meshFragmentShaderFilePath;
    Params.meshFilePath = &meshFilePath;
    Params.textureFilePath = &textureFilePath;
    Params.initialPosition = Math::sVector(0.0f, 0.0f, 0.0f);
    Params.maxVelocity = 10.0f;

    cGameObject* gameObject = nullptr;
    if (!(result = cGameObject::Create(gameObject, Params)))
    {
        EAE6320_ASSERT(false);
        goto OnExit;
    }
    else
    {
        m_gameObjectList.push_back(gameObject);
    }

    result = InitializeRings();

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeRings()
{
    cResult result = Results::Success;

    static const std::string meshFilePath = std::string("data/Meshes/Ring.msh");
    static const std::string textureFilePath = std::string("data/Textures/Ring.tex");

    sGameObjectinitializationParameters Params;
    Params.vertexShaderFilePath = &s_meshVertexShaderFilePath;
    Params.fragmentShaderFilePath = &s_meshFragmentShaderFilePath;
    Params.meshFilePath = &meshFilePath;
    Params.textureFilePath = &textureFilePath;

    constexpr uint8_t numCoins = 100;
    constexpr float maxX = 2.0f;
    constexpr float maxY = 2.0f;
    constexpr float maxZ = 75.0f;

    for (uint8_t i = 0; i < numCoins; ++i)
    {
        const float randX = Math::RandRange(-maxX, maxX);
        const float randY = Math::RandRange(-maxY, maxY);

        Params.initialPosition = Math::sVector(randX, randY, -maxZ - i * maxZ);

        cGameObject* gameObject = nullptr;
        if (!(result = cGameObject::Create(gameObject, Params)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_gameObjectList.push_back(gameObject);
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeSkyBox()
{
    cResult result = Results::Success;

    if (!(result = Graphics::cEffect::Create(m_skyBoxEffect, s_meshVertexShaderFilePath.c_str(), s_meshFragmentShaderFilePath.c_str(), 0)))
    {
        EAE6320_ASSERT(false);
        goto OnExit;
    }


    if (!(result = Graphics::cTexture::s_manager.Load("data/Textures/SkyBox.tex", m_skyBoxTexture)))
    {
        EAE6320_ASSERT(false);
        goto OnExit;
    }

    if (!(result = Graphics::cMesh::s_manager.Load("data/Meshes/SkyBox.msh", m_skyBoxMesh)))
    {
        EAE6320_ASSERT(false);
        goto OnExit;
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
