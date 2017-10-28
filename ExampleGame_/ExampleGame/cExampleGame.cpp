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
const std::string eae6320::cExampleGame::s_spriteVertexShaderFilePath("data/Shaders/Vertex/sprite.shd");
const std::string eae6320::cExampleGame::s_spriteFragmentShaderFilePath("data/Shaders/Fragment/spriteBasic.shd");
const std::string eae6320::cExampleGame::s_animatedSpriteFragmentShaderFilePath("data/Shaders/Fragment/spriteAnimated.shd");
const std::string eae6320::cExampleGame::s_textureFolderList[s_numTextureFolders] = { "data/Textures/Dust/", "data/Textures/Forest/", "data/Textures/Ramps/" };

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
    // Animate the background
    {
        static float systemTime = 0.0f;
        systemTime += i_elapsedSecondCount_sinceLastUpdate;
        m_backgroundColor.r = 1.0f + (cosf(systemTime) * 0.5f) - 0.5f;
        m_backgroundColor.g = 1.0f + (sinf(systemTime) * 0.5f) - 0.5f;
        m_backgroundColor.b = 1.0f + (acosf(systemTime) * 0.5f) - 0.5f;
    }
}

void eae6320::cExampleGame::UpdateSimulationBasedOnInput()
{
    m_isUpPressed = UserInput::IsKeyPressed(UserInput::KeyCodes::Up);
    m_isDownPressed = UserInput::IsKeyPressed(UserInput::KeyCodes::Down);
    m_isLeftPressed = UserInput::IsKeyPressed(UserInput::KeyCodes::Left);
    m_isRightPressed = UserInput::IsKeyPressed(UserInput::KeyCodes::Right);
}

void eae6320::cExampleGame::UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
    UpdateGameObjects(i_elapsedSecondCount_sinceLastUpdate);
    UpdateSpriteRenderData(i_elapsedSecondCount_sinceLastUpdate);
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

    for (auto& mesh : m_meshList)
    {
        Graphics::SubmitMeshToBeRendered(mesh, m_effectList[0], Math::sVector(), Math::cQuaternion());
    }

    for (auto& spriteRenderData : m_spriteRenderDataList)
    {
        Graphics::cTexture* texture = Graphics::cTexture::s_manager.Get(m_textureList[spriteRenderData.m_currentFrameIndex]);
        Graphics::SubmitSpriteToBeRendered(spriteRenderData.m_sprite, spriteRenderData.m_effect, texture);
    }

    for (auto& gameObject : m_gameObjectList)
    {
        gameObject->SubmitDataToBeRendered(i_elapsedSecondCount_systemTime, i_elapsedSecondCount_sinceLastSimulationUpdate);
    }
}

void eae6320::cExampleGame::UpdateGameObjects(const float i_elapsedSecondCount_sinceLastUpdate)
{
    {
        static const float impulseMagnitude = 2.0f;
        Math::sVector impulse;
        impulse.x = m_isLeftPressed ? -impulseMagnitude : 0.0f + m_isRightPressed ? impulseMagnitude : 0.0f;
        impulse.y = m_isDownPressed ? -impulseMagnitude : 0.0f + m_isUpPressed ? impulseMagnitude : 0.0f;

        m_gameObjectList[0]->AddImpulse(impulse);
    }

    {
        //m_camera.m_rigidBodyState.velocity = impulse;
        //m_camera.m_rigidBodyState.Update(i_elapsedSecondCount_sinceLastUpdate);
    }

    for (const auto& gameObject : m_gameObjectList)
    {
        gameObject->UpdateBasedOnTime(i_elapsedSecondCount_sinceLastUpdate);
    }
}

void eae6320::cExampleGame::UpdateSpriteRenderData(const float i_elapsedSecondCount_sinceLastUpdate)
{
    // Swap textures based on time.
    {
        for (auto& spriteRenderData : m_spriteRenderDataList)
        {
            spriteRenderData.m_waitUntilNextFrame -= i_elapsedSecondCount_sinceLastUpdate;
            if (spriteRenderData.m_waitUntilNextFrame < 0.0f)
            {
                ++spriteRenderData.m_currentFrameIndex;
                spriteRenderData.m_currentFrameIndex = spriteRenderData.m_currentFrameIndex - spriteRenderData.m_firstFrameIndex >= s_numFrames ? spriteRenderData.m_firstFrameIndex : spriteRenderData.m_currentFrameIndex;
                spriteRenderData.m_waitUntilNextFrame = spriteRenderData.m_frameRate;
            }
        }
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

    // Initialize sprites
    if (!(result = InitializeSprites()))
    {
        goto OnExit;
    }

    InitializeSpriteRenderDataList();

    // Initialize game objects
    if (!(result = InitializeGameObjects()))
    {
        goto OnExit;
    }

    m_camera.m_rigidBodyState.position.z = 10;

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

    for (const auto& mesh : m_meshList)
    {
        mesh->DecrementReferenceCount();
    }
    m_meshList.clear();

    for (const auto& sprite : m_spriteList)
    {
        sprite->DecrementReferenceCount();
    }
    m_spriteList.clear();

    m_spriteRenderDataList.clear();

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

    constexpr uint8_t numEffects = 3;
    m_effectList.reserve(numEffects);

    // initialize the mesh effect
    {
        Graphics::cEffect* effect = nullptr;

        if (!(result = Graphics::cEffect::Create(effect, s_meshVertexShaderFilePath.c_str(), s_meshFragmentShaderFilePath.c_str(), Graphics::RenderStates::AlphaTransparency | Graphics::RenderStates::DepthBuffering)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_effectList.push_back(effect);
        }
    }

    // initialize the simple sprite effect
    {
        Graphics::cEffect* effect = nullptr;

        if (!(result = Graphics::cEffect::Create(effect, s_spriteVertexShaderFilePath.c_str(), s_spriteFragmentShaderFilePath.c_str(), Graphics::RenderStates::AlphaTransparency)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_effectList.push_back(effect);
        }
    }

    // initialize the animated sprite effect
    {
        Graphics::cEffect* effect = nullptr;

        if (!(result = Graphics::cEffect::Create(effect, s_spriteVertexShaderFilePath.c_str(), s_animatedSpriteFragmentShaderFilePath.c_str(), Graphics::RenderStates::AlphaTransparency)))
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

    constexpr uint8_t numTextures = s_numTextureFolders * s_numFrames;
    m_textureList.reserve(numTextures);

    static const std::string textureNameSuffix("frame_");
    static const std::string textureFileExtension(".tex");

    for (const auto& textureFolder : s_textureFolderList)
    {
        const std::string textureBaseName(textureFolder + textureNameSuffix);
        for (uint8_t i = 0; i < s_numFrames; ++i)
        {
            const std::string textureName = textureBaseName + std::to_string(i) + textureFileExtension;
            Graphics::cTexture::Handle textureHandle;
            if (!(result = Graphics::cTexture::s_manager.Load(textureName.c_str(),
                textureHandle)))
            {
                EAE6320_ASSERT(false);
                goto OnExit;
            }
            else
            {
                m_textureList.push_back(textureHandle);
            }
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeMeshes()
{
    cResult result = Results::Success;

    {
        constexpr uint16_t vertexCount = 6;

        const Math::sVector vertices[vertexCount] = {
            { -2.0f, -1.0f, 2.0f }, { 2.0f, -1.0f, 2.0f }, { 2.0f, -1.0f, -2.0f },
            { -2.0f, -1.0f, 2.0f }, { 2.0f, -1.0f, -2.0f }, { -2.0f, -1.0f, -2.0f }
        };

        const uint16_t indices[vertexCount] = { 0, 1, 2, 3, 4, 5 };

        const Graphics::sColor colors[vertexCount] = {
            Graphics::sColor::BLACK, Graphics::sColor::BLACK, Graphics::sColor::SILVER,
            Graphics::sColor::BLACK, Graphics::sColor::SILVER, Graphics::sColor::SILVER,
        };

        Graphics::cMesh* mesh = nullptr;
        if (!(result = Graphics::cMesh::Create(mesh, vertexCount, vertices, indices, colors)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_meshList.push_back(mesh);
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeSprites()
{
    cResult result = Results::Success;

    const Math::sVector2d origins[s_numTextureFolders] = { { 0.0f, -0.5f }, { -0.4f, 0.5f }, { -0.75f, 0.75f } };
    const Math::sVector2d extents[s_numTextureFolders] = { { 0.75f, 0.35f }, { 0.4f, 0.3f } , { 0.3f, 0.3f } };

    m_spriteList.reserve(s_numTextureFolders);

    for (uint8_t i = 0; i < s_numTextureFolders; ++i)
    {
        Graphics::cSprite* sprite = nullptr;

        if (!(result = Graphics::cSprite::Create(sprite, origins[i], extents[i])))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_spriteList.push_back(sprite);
        }
    }

OnExit:

    return result;
}

void eae6320::cExampleGame::InitializeSpriteRenderDataList()
{
    const size_t numSprites = m_spriteList.size();
    //for (uint8_t i = 0; i < numSprites; ++i)
    const uint8_t i = static_cast<const uint8_t>(numSprites) - 1;
    {
        sSpriteRenderData spriteRenderData;
        {
            spriteRenderData.m_firstFrameIndex = i * s_numFrames;
            spriteRenderData.m_currentFrameIndex = i * s_numFrames;
            spriteRenderData.m_frameRate = 0.125f + float(i) * 0.05f;
            spriteRenderData.m_waitUntilNextFrame = spriteRenderData.m_frameRate;
            spriteRenderData.m_effect = m_effectList[1];
            spriteRenderData.m_sprite = m_spriteList[i];
        }
        m_spriteRenderDataList.push_back(spriteRenderData);
    }
}

eae6320::cResult eae6320::cExampleGame::InitializeGameObjects()
{
    cResult result = Results::Success;

    cGameObject* gameObject = nullptr;

    if (!(result = cGameObject::Create(gameObject, Math::sVector(), Graphics::sColor::RED, Graphics::sColor::YELLOW)))
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
