// Include Files
//==============

#include "cExampleGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cSprite.h>
#include <Engine/Graphics/Graphics.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/sVector2d.h>
#include <Engine/UserInput/UserInput.h>

#include <ctime>
#include <string>

// Static Data Initialization
//===========================

const std::string eae6320::cExampleGame::s_vertexShaderFilePath("data/Shaders/Vertex/sprite.shd");
const std::string eae6320::cExampleGame::s_simpleFragmentShaderFilePath("data/Shaders/Fragment/spriteBasic.shd");
const std::string eae6320::cExampleGame::s_animatedFragmentShaderFilePath("data/Shaders/Fragment/spriteAnimated.shd");
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

    m_swapSpritesBasedOnInput = UserInput::IsKeyPressed(UserInput::KeyCodes::Space);
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

    UpdateSpriteRenderData(i_elapsedSecondCount_sinceLastUpdate);
}

void eae6320::cExampleGame::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
    eae6320::Graphics::SubmitBackgroundColor(m_backgroundColor);

    for (auto& spriteRenderData : m_spriteRenderDataList)
    {
        eae6320::Graphics::cTexture* texture = eae6320::Graphics::cTexture::s_manager.Get(m_textureList[spriteRenderData.m_currentFrameIndex]);
        eae6320::Graphics::SubmitDataToBeRendered(spriteRenderData.m_sprite, spriteRenderData.m_effect, texture);
    }
}

void eae6320::cExampleGame::UpdateSpriteRenderData(const float i_elapsedSecondCount_sinceLastUpdate)
{
    // Swap effects based on user input
    {
        m_spriteRenderDataList[0].m_effect = m_swapSpritesBasedOnInput ? m_effectList[1] : m_effectList[0];
    }

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
    eae6320::cResult result = eae6320::Results::Success;

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

    // Initialize sprites
    if (!(result = InitializeSprites()))
    {
        goto OnExit;
    }

    InitializeSpriteRenderDataList();

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::CleanUp()
{
    eae6320::cResult result = eae6320::Results::Success;

    for (const auto& effect : m_effectList)
    {
        effect->DecrementReferenceCount();
    }
    m_effectList.clear();

    for (auto& texture : m_textureList)
    {
        const auto localResult = eae6320::Graphics::cTexture::s_manager.Release(texture);
        if (!localResult)
        {
            EAE6320_ASSERT(false);
            if (result)
            {
                result = localResult;
            }
        }
    }

    for (const auto& sprite : m_spriteList)
    {
        sprite->DecrementReferenceCount();
    }
    m_spriteList.clear();

    m_spriteRenderDataList.clear();

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeEffects()
{
    eae6320::cResult result = eae6320::Results::Success;

    constexpr uint8_t numEffects = 2;
    m_effectList.reserve(numEffects);

    // initialize the simple effect
    {
        eae6320::Graphics::cEffect* effect = nullptr;

        if (!(result = eae6320::Graphics::cEffect::Load(effect, s_vertexShaderFilePath.c_str(), s_simpleFragmentShaderFilePath.c_str())))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_effectList.push_back(effect);
        }
    }

    // initialize the animated effect
    {
        eae6320::Graphics::cEffect* effect = nullptr;

        if (!(result = eae6320::Graphics::cEffect::Load(effect, s_vertexShaderFilePath.c_str(), s_animatedFragmentShaderFilePath.c_str())))
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
    eae6320::cResult result = eae6320::Results::Success;

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
            eae6320::Graphics::cTexture::Handle textureHandle;
            if (!(result = eae6320::Graphics::cTexture::s_manager.Load(textureName.c_str(),
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

eae6320::cResult eae6320::cExampleGame::InitializeSprites()
{
    eae6320::cResult result = eae6320::Results::Success;

    const eae6320::Math::sVector2d origins[3] = { { 0.0f, -0.5f }, { -0.5f, 0.5f }, { 0.6f, 0.5f } };
    const eae6320::Math::sVector2d extents[3] = { { 0.75f, 0.35f }, { 0.4f, 0.3f } , { 0.3f, 0.3f } };

    m_spriteList.reserve(s_numTextureFolders);

    for (uint8_t i = 0; i < s_numTextureFolders; ++i)
    {
        eae6320::Graphics::cSprite* sprite = nullptr;

        if (!(result = eae6320::Graphics::cSprite::Create(sprite, origins[i], extents[i])))
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
    for (uint8_t i = 0; i < numSprites; ++i)
    {
        sSpriteRenderData spriteRenderData;
        {
            spriteRenderData.m_firstFrameIndex = i * s_numFrames;
            spriteRenderData.m_currentFrameIndex = i * s_numFrames;
            spriteRenderData.m_frameRate = 0.125f + float(i) * 0.05f;
            spriteRenderData.m_waitUntilNextFrame = spriteRenderData.m_frameRate;
            spriteRenderData.m_effect = m_effectList[0];
            spriteRenderData.m_sprite = m_spriteList[i];
        }
        m_spriteRenderDataList.push_back(spriteRenderData);
    }
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
