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
        //m_camera.m_rigidBodyState.velocity.x = eae6320::Math::Clamp<float>(m_camera.m_rigidBodyState.velocity.x, -cGameObject::s_maxVelocity, cGameObject::s_maxVelocity);
        //m_camera.m_rigidBodyState.velocity.y = eae6320::Math::Clamp<float>(m_camera.m_rigidBodyState.velocity.y, -cGameObject::s_maxVelocity, cGameObject::s_maxVelocity);
        //m_camera.m_rigidBodyState.velocity.z = eae6320::Math::Clamp<float>(m_camera.m_rigidBodyState.velocity.z, -cGameObject::s_maxVelocity, cGameObject::s_maxVelocity);
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
    UpdateCoins(i_elapsedSecondCount_sinceLastUpdate);
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

    m_camera.m_rigidBodyState.position.y = 2.5f;
    m_camera.m_rigidBodyState.position.z = 15.0f;

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::CleanUp()
{
    cResult result = Results::Success;

    for (auto& gameObject : m_gameObjectList)
    {
        cGameObject::Destroy(gameObject);
    }
    m_gameObjectList.clear();

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeGameObjects()
{
    cResult result = Results::Success;

    static const std::string meshFilePath = std::string("data/Meshes/Ship.msh");
    static const std::string textureFilePath = std::string("data/Textures/Soccer/Wood.tex");

    sGameObjectinitializationParameters Params;
    Params.vertexShaderFilePath = &s_meshVertexShaderFilePath;
    Params.fragmentShaderFilePath = &s_meshFragmentShaderFilePath;
    Params.meshFilePath = &meshFilePath;
    Params.textureFilePath = &textureFilePath;
    Params.initialPosition = Math::sVector(0.0f, 0.0f, 0.0f);
    Params.maxVelocity = 5.0f;

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

    result = InitializeCoins();

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeCoins()
{
    cResult result = Results::Success;

    static const std::string meshFilePath = std::string("data/Meshes/Coin.msh");
    static const std::string textureFilePath = std::string("data/Textures/Soccer/Wood.tex");

    sGameObjectinitializationParameters Params;
    Params.vertexShaderFilePath = &s_meshVertexShaderFilePath;
    Params.fragmentShaderFilePath = &s_meshFragmentShaderFilePath;
    Params.meshFilePath = &meshFilePath;
    Params.textureFilePath = &textureFilePath;
    Params.maxVelocity = 10.0f;

    constexpr uint8_t numCoins = 5;
    constexpr float maxX = 10.0f;
    constexpr float maxY = 10.0f;
    constexpr float maxZ = 50.0f;

    for (uint8_t i = 0; i < numCoins; ++i)
    {
        const float randX = Math::RandRange(-maxX, maxX);
        const float randY = Math::RandRange(-maxY, maxY);

        Params.initialPosition = Math::sVector(randX, randY, -maxZ);

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
