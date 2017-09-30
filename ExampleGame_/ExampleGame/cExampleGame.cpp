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

// Static Data Initialization
//===========================

const std::string eae6320::cExampleGame::s_vertexShaderFilePath("data/Shaders/Vertex/sprite.shd");
const std::string eae6320::cExampleGame::s_simpleFragmentShaderFilePath("data/Shaders/Fragment/spriteBasic.shd");
const std::string eae6320::cExampleGame::s_animatedFragmentShaderFilePath("data/Shaders/Fragment/spriteAnimated.shd");
const std::string eae6320::cExampleGame::s_textureFolderPath("data/Textures/");

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

    UpdateSpriteRenderData();

    if (m_spriteSwapTicker > 0.0f)
    {
        m_spriteSwapTicker -= i_elapsedSecondCount_sinceLastUpdate;
        if (m_spriteSwapTicker < 0.0f)
        {
            m_swapSpritesBasedOnTime = !m_swapSpritesBasedOnTime;
            m_spriteSwapTicker = m_spriteSwapInterval;
        }
    }
}

void eae6320::cExampleGame::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
    eae6320::Graphics::SubmitBackgroundColor(m_backgroundColor);

    for (auto& spriteRenderData : m_spriteRenderDataList)
    {
        eae6320::Graphics::SubmitDataToBeRendered(spriteRenderData.m_sprite, spriteRenderData.m_effect, spriteRenderData.m_texture);
    }
}

void eae6320::cExampleGame::UpdateSpriteRenderData()
{
    // Swap textures based on user input.
    {
        eae6320::Graphics::cTexture::Handle textureToUse = m_swapSpritesBasedOnInput ? m_textureList[1] : m_textureList[0];
        m_spriteRenderDataList[0].m_texture = textureToUse;
        m_spriteRenderDataList[m_numColumns - 1].m_texture = textureToUse;
        m_spriteRenderDataList[m_numColumns * (m_numRows - 1)].m_texture = textureToUse;
        m_spriteRenderDataList[m_numColumns * m_numRows - 1].m_texture = textureToUse;

        // This one is special
        m_spriteRenderDataList[m_numColumns * m_numRows / 2].m_texture = m_swapSpritesBasedOnInput ? m_textureList[2] : m_textureList[0];
    }

    // Swap effects based on user input
    {
        m_spriteRenderDataList[m_numColumns * m_numRows / 2].m_effect = m_swapSpritesBasedOnInput ? m_effectList[1] : m_effectList[0];
    }

    // Swap textures based on time.
    {
        eae6320::Graphics::cTexture::Handle textureToUse = m_swapSpritesBasedOnTime ? m_textureList[1] : m_textureList[3];
        m_spriteRenderDataList[m_numColumns / 2].m_texture = textureToUse;
        m_spriteRenderDataList[m_numColumns * (m_numRows / 2)].m_texture = textureToUse;
        m_spriteRenderDataList[m_numColumns * (m_numRows / 2) + m_numColumns - 1].m_texture = textureToUse;
        m_spriteRenderDataList[m_numColumns * m_numRows - m_numColumns / 2 - 1].m_texture = textureToUse;
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

    constexpr uint8_t numTextures = 4;
    m_textureList.reserve(numTextures);

    const std::string africaTwinTextureBaseName(s_textureFolderPath + std::string("africa_twin_0"));
    const std::string textureFileExtension(".tex");

    // initialize the four africa twin textures
    for (uint8_t i = 0; i < numTextures; ++i)
    {
        const std::string africaTwinTextureName = africaTwinTextureBaseName + std::to_string(i + 1) + textureFileExtension;

        eae6320::Graphics::cTexture::Handle africaTwinTexture;
        if (!(result = eae6320::Graphics::cTexture::s_manager.Load(africaTwinTextureName.c_str(),
            africaTwinTexture)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_textureList.push_back(africaTwinTexture);
        }
    }

OnExit:

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeSprites()
{
    eae6320::cResult result = eae6320::Results::Success;

    const uint8_t numSprites = m_numColumns * m_numRows;
    constexpr float size = 0.1f;
    const float offset = size * float(m_numColumns * 2 - 2);
    const eae6320::Math::sVector2d extents(size, size);

    m_spriteList.reserve(numSprites);

    for (uint8_t i = 0; i < numSprites; ++i)
    {
        eae6320::Graphics::cSprite* sprite = nullptr;

        eae6320::Math::sVector2d origin(i % m_numColumns * size * 4 - offset, i / m_numColumns * size * 4 - offset);

        if (!(result = eae6320::Graphics::cSprite::Create(sprite, origin, extents)))
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
    for (size_t i = 0; i < numSprites; ++i)
    {
        sSpriteRenderData spriteRenderData;
        {
            spriteRenderData.m_effect = m_effectList[0];
            spriteRenderData.m_texture = m_textureList[0];
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
