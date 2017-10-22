// Include Files
//==============

#include "cExampleGame.h"

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
    //eae6320::Graphics::SubmitBackgroundColor(m_backgroundColor);
    eae6320::Graphics::SubmitBackgroundColor(eae6320::Graphics::sColor::EMERALD);

    for (auto& mesh : m_meshList)
    {
        static Math::sVector position;
        eae6320::Graphics::SubmitMeshToBeRendered(mesh, m_effectList[0], position);
    }

    //for (auto& spriteRenderData : m_spriteRenderDataList)
    //{
    //    eae6320::Graphics::cTexture* texture = eae6320::Graphics::cTexture::s_manager.Get(m_textureList[spriteRenderData.m_currentFrameIndex]);
    //    eae6320::Graphics::SubmitSpriteToBeRendered(spriteRenderData.m_sprite, spriteRenderData.m_effect, texture);
    //}

    //for (auto& widget : m_widgetList)
    //{
    //    widget->SubmitDataToBeRendered(i_elapsedSecondCount_systemTime, i_elapsedSecondCount_sinceLastSimulationUpdate);
    //}
}

void eae6320::cExampleGame::UpdateSpriteRenderData(const float i_elapsedSecondCount_sinceLastUpdate)
{
    // Swap effects based on user input
    {
        m_spriteRenderDataList[0].m_effect = m_swapSpritesBasedOnInput ? m_effectList[2] : m_effectList[1];
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

    // Initialize widgets
    if (!(result = InitializeWidgets()))
    {
        goto OnExit;
    }

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

    for (const auto& widget : m_widgetList)
    {
        widget->DecrementReferenceCount();
    }
    m_widgetList.clear();

    return result;
}

eae6320::cResult eae6320::cExampleGame::InitializeEffects()
{
    eae6320::cResult result = eae6320::Results::Success;

    constexpr uint8_t numEffects = 3;
    m_effectList.reserve(numEffects);

    // initialize the mesh effect
    {
        eae6320::Graphics::cEffect* effect = nullptr;

        if (!(result = eae6320::Graphics::cEffect::Create(effect, s_meshVertexShaderFilePath.c_str(), s_meshFragmentShaderFilePath.c_str())))
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
        eae6320::Graphics::cEffect* effect = nullptr;

        if (!(result = eae6320::Graphics::cEffect::Create(effect, s_spriteVertexShaderFilePath.c_str(), s_spriteFragmentShaderFilePath.c_str())))
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
        eae6320::Graphics::cEffect* effect = nullptr;

        if (!(result = eae6320::Graphics::cEffect::Create(effect, s_spriteVertexShaderFilePath.c_str(), s_animatedSpriteFragmentShaderFilePath.c_str())))
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

eae6320::cResult eae6320::cExampleGame::InitializeMeshes()
{
    eae6320::cResult result = eae6320::Results::Success;

    {
        constexpr uint16_t vertexCount = 3;
        
        const eae6320::Math::sVector vertices[vertexCount] = { 
            { 0.0f, 0.0f, 0.0f },
            { 0.25f, 0.0f, 0.0f },
            { 0.125f, 0.25f, 0.0f }
        };

        const uint16_t indices[vertexCount] = { 0, 1, 2 };

        const eae6320::Graphics::sColor colors[vertexCount] = { 
            eae6320::Graphics::sColor::RED, 
            eae6320::Graphics::sColor::GREEN,
            eae6320::Graphics::sColor::BLUE
        };

        eae6320::Graphics::cMesh* mesh = nullptr;
        if (!(result = eae6320::Graphics::cMesh::Create(mesh, vertexCount, vertices, indices, colors)))
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
    eae6320::cResult result = eae6320::Results::Success;

    const eae6320::Math::sVector2d origins[s_numTextureFolders] = { { 0.0f, -0.5f }, { -0.4f, 0.5f }, { 0.5f, 0.5f } };
    const eae6320::Math::sVector2d extents[s_numTextureFolders] = { { 0.75f, 0.35f }, { 0.4f, 0.3f } , { 0.3f, 0.3f } };

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
            spriteRenderData.m_effect = m_effectList[1];
            spriteRenderData.m_sprite = m_spriteList[i];
        }
        m_spriteRenderDataList.push_back(spriteRenderData);
    }
}

eae6320::cResult eae6320::cExampleGame::InitializeWidgets()
{
    eae6320::cResult result = eae6320::Results::Success;

    eae6320::UserInterface::cWidget::sInitializationParameters params;
    params.vertexShaderName = eae6320::cExampleGame::s_spriteVertexShaderFilePath.c_str();
    params.fragmentShaderName = eae6320::cExampleGame::s_spriteFragmentShaderFilePath.c_str();
    params.scale = { 0.25f, 0.25f };

    const std::string texturePath = std::string("data/Textures/Arrows/frame_");
    const std::string textureFileExtension(".tex");

    constexpr uint8_t numArrows = 4;
    const eae6320::Math::sVector2d positions[numArrows] = { { 1.0f, 1.0f }, { 1.0f, -1.0f }, { -1.0f, -1.0f }, { -1.0f, 1.0f } };
    const eae6320::Math::sVector2d anchors[numArrows] = { { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f } };
    
    m_widgetList.reserve(numArrows);

    for (uint8_t i = 0; i < numArrows; ++i)
    {
        std::string textureName = texturePath + std::to_string(i) + textureFileExtension;
        params.textureName = textureName.c_str();
        params.position = positions[i];
        params.anchor = anchors[i];

        eae6320::UserInterface::cWidget* widget = nullptr;

        if (!(result = eae6320::UserInterface::cWidget::Create(widget, params)))
        {
            EAE6320_ASSERT(false);
            goto OnExit;
        }
        else
        {
            m_widgetList.push_back(widget);
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
